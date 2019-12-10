/* 
Contains three classes
	Mesh Manager, to export all the mesh information into map files
	Material Manager, to export all material info and maps into material file
	Collider Manager, to generate colliders depending on our needs.
*/
filein "mvd_json_exporter.ms"
filein "mvd_components.ms"
filein "mvd_utils.ms"

-- Determine geometry layer names and other definitions in this file header
global layer_colliders = "colliders"

struct TMeshSubGroup (
	first_idx,
	num_indices,
	material_index,
	user_material_id
)

-- Class used to export the meshes as .obj or binary files
struct MeshManager 
(
	fs = MVDJson(),           
	vtxs = #(),
	idxs = #(),
	subgroups = #(),
	type = "PosNUvUvT",
	
	-- Method to export mesh in .obj format
	fn exportItem obj epath = 
	(
		-- Build the path and export file
		select obj
		local full_path = (epath + obj.name + ".obj")
		exportFile full_path #noPrompt selectedOnly:true #noPrompt  
		return full_path
	),
	
	fn exportBinaryFetch obj = (
	
		-- The final list of vtxs and indexes to be exported
		local subgroups = #()
			
		-- An array of arrays, each containing the list of faces using the material nth
		local faces_by_material_used = getMaterialsUsedByMesh obj
		
		local MaxToMVD = rotateXMatrix  -90
		local MaxToLocal = inverse obj.transform
		local MaxToLocalMVD = MaxToLocal * MaxToMVD
		
		-- To quickly find if a vertex is new
		local HT = dotNetObject "System.Collections.Hashtable"
		local num_faces = getNumFaces obj
		
		-- Primary and secondary map support
		local mapA = 1
		local mapB = mapA
		local mapB_status = meshop.getMapSupport obj 3
		if mapB_status == true then ( mapB = 3 )
			
		-- For each material used by the mesh
		local mat_idx 
		for mat_idx = 1 to faces_by_material_used.count do (
			local faces = faces_by_material_used[mat_idx]
			if faces == undefined then continue

			-- We are starting a new group. 
			local new_subgroup = TMeshSubGroup()
			new_subgroup.first_idx = idxs.count
			
			-- Each face uses 3 indices..
			new_subgroup.num_indices = faces.count * 3
			new_subgroup.material_index = subgroups.count
			new_subgroup.user_material_id = mat_idx
			
			-- For each face using that material
			for face_id in faces do (
			
				-- Retrieve the 3 indexes in the list of vertex positions
				local face = getFace obj face_id
				local faceA = meshop.getMapFace obj mapA face_id
				local faceB = meshop.getMapFace obj mapB face_id
				local normals = meshop.getFaceRNormals obj face_id
				
				-- For each vertex of each face
				local vtx_id
				for vtx_id in #(1,3,2) do (
					
					-- Get the real vertex position in local coordinates
					local posW = getVert obj face[ vtx_id ]
					local pos = posW * MaxToLocalMVD
					
					-- Get normals
					local NW = normals[ vtx_id ]
					local N = NW * MaxToLocalMVD.rotationpart
					
					-- Get UV's
					local uvA = meshop.getMapVert obj mapA faceA[vtx_id]
					local uvB = meshop.getMapVert obj mapB faceB[vtx_id]
					
					-- Buffer with the information
					local new_vtx = #( pos.x, pos.y, pos.z, N.x, N.y, N.z, uvA.x, 1-uvA.y, uvB.x, 1-uvB.y ) 

					-- Check if the vtx is unique, get the final index then
					local new_idx = findIndexOrRegister ht vtxs new_vtx
					
					-- Register the index
					append idxs new_idx
				)
			)
			
			append subgroups new_subgroup
		)

		computeTangentSpace vtxs idxs
	),
	
	fn exportBinaryFile obj epath = (

			print epath
		-- Chunk = MagicID(4bytes) + Size(4bytes)
		local magicHeader = 0x44444444	-- Chunk HeaderData
		local magicVtxs = 0x55554433		-- Chunk Vtxs
		local magicIdxs = 0x55556677		-- Chunk Idxs
		local magicSubGroups = 0x55556688
		local magicEoF = 0x55558888		-- .. Chunk CollisionData
		local triangle_list = 4
		
		-- Create file
		local fs = fopen epath "wb"
		local bytes_per_vtx = vtxs[1].count * 4
		local bytes_per_idx = 4 -- up to 65536
		
		-- Total Bytes, NumIdxs, 
		local headerSize = 4*(4+4)+32
		writeChunk fs magicHeader headerSize	-- 8 bytes
		
		WriteLong fs vtxs.count		-- NumVertexs
		WriteLong fs idxs.count		-- NumIndices
		WriteLong fs triangle_list  -- PrimitiveType
		WriteLong fs bytes_per_vtx  -- bytes_per_vtx
		
		WriteLong fs bytes_per_idx  -- bytes_per_idx
		WriteLong fs subgroups.count   -- Num SubGroups
		WriteLong fs 0              -- dummy2, AABB?
		WriteLong fs 0              -- dummy3

		-- Write the vertex type as 32 bytes string with zero paddings
		WriteString fs type
		for i = type.count+1 to 31 do WriteByte fs 0
		
		-- Save vtxs
		local bytes_vtxs = vtxs.count * bytes_per_vtx
		writeChunk fs magicVtxs bytes_vtxs
		for v in vtxs do (	-- Each vtx...
			for vd in v do (	-- Each component in the vtxs
				writeFloat fs vd
			)
		)
		
		-- Save idxs, header + data
		local bytes_idxs = idxs.count * bytes_per_idx
		writeChunk fs magicIdxs bytes_idxs
		for v in idxs do (	-- Each idxs...
			local v0 = v - 1  -- Because max starts at 1, but us at 0
			writeLong fs v0 
		)
		
		-- Save subgroups, header + data
		local bytes_per_subgroup = 4 * 4			-- 4 ints
		local bytes_subgroups = subgroups.count * bytes_per_subgroup
		writeChunk fs magicSubGroups bytes_subgroups
		for sg in subgroups do (	-- Each subgroup
			writeLong fs sg.first_idx #unsigned
			writeLong fs sg.num_indices #unsigned
			writeLong fs sg.material_index #unsigned
			writeLong fs sg.user_material_id #unsigned
		)
		
		-- end of file and close
		writeChunk fs magicEoF 0		
		fclose fs
	),
	
	-- Method to export a mesh in binary format
	fn exportItemBinary obj epath = (
		
		-- Convert the given object to triangular mesh
		-- Make a deep copy of it to prevent modifiyng the original one
		local given_obj = obj
		if classof obj != TriMesh then (
			given_obj = copy obj
			given_obj.name = obj.name
			convertToMesh given_obj
		)
		
		-- Get all the mesh information from the object
		exportBinaryFetch given_obj
		
		-- Export the object into a binary file
		exportBinaryFile given_obj (epath + obj.name + ".mesh")
		
		-- Remove copied item if necessary
		if given_obj != obj then delete given_obj
		
		return (epath + obj.name + ".mesh")
	)
)

-- Class used to export materials and textures
struct MaterialManager (
	
	fs = MVDJson(),    
	fs_tex = MVDJson(), 	
	tech = "pbr.tech",
	shadows = true,
	gen_mat = true,
	gen_tex = true,
	
	-- Export the texture of the given map
	fn exportMap emap ename = (
		
		-- Build the json and export the texture
		local f_name = filenameFromPath emap.filename
		local full_path = (tex_path + f_name)
		fs_tex.add ename full_path
		
		-- Check if full_path exists, if not, copy texture there
		if not doesFileExist full_path and gen_tex == true then (
			format "Copying file from % to %\n" emap.filename full_path
			copyFile emap.filename full_path
		)
	),
	
	-- Export the material given, parameters and texture definition
	fn exportItem emat epath = (
		
		fs.add "tech" tech
		fs.add "shadows" (shadows as integer)
		local short_path = (epath + emat.name + ".mtl")
		local full_path = (short_path)
		
		-- Atleast default diffuse needed
		if emat.diffusemap != undefined then (
			exportMap emat.diffusemap "diffuse"
		) else (
			fs_tex.add "diffuse" (tex_path + "default.png")
		)
		
		-- Atleast default normal needed
		if emat.bumpMap != undefined then (
			exportMap emat.bumpMap "normal"
		) else (
			fs_tex.add "normal" (tex_path + "null_normal.jpg")
		)
		
		if emat.ambientMap != undefined then (
			exportMap emat.ambientMap "lightmap"
		)		
		
		if emat.specularMap != undefined then (
			exportMap emat.specularMap "specular"
		)	
		
		if emat.glossinessMap != undefined then (
			exportMap emat.glossinessMap "roughness"
		)	
		
		if emat.displacementMap != undefined then (
			exportMap emat.displacementMap "height"
		)
		
		if emat.selfIllumMap != undefined then (
			exportMap emat.selfIllumMap "emissive"
		)
		
		if emat.refractionMap != undefined then (
			exportMap emat.refractionMap "aocclusion"
		)
		
		fs.add "textures" fs_tex
		if gen_mat == true then (fs.writeDisk full_path)
		
		return (mats_path + emat.name + ".mtl")
	)
)

-- Collider manager
-- Used to generate collider meshes and types
struct ColliderManager 
(	
	-- Basic mesh generation
	fn generateBasic obj = (
		
		addComponent obj "collider"
	),
	
	
	-- Convex mesh generation
	fn generateConvex obj = (
		
		col_layer = getLayerByName layer_colliders
		
		local cvx_hull = nvpx.CreateConvexHull obj.mesh 100 0
		local cloned_mesh = box name:"cloned_collider"
		local conv_obj = convertToMesh cloned_mesh
		cloned_mesh.mesh = cvx_hull
		cloned_mesh.rotation = obj.rotation
		cloned_mesh.pos = obj.pos
		cloned_mesh.name = "collider_" + obj.name
		cloned_mesh.parent = obj
		
		col_layer.addnode cloned_mesh
		addComponent obj "collider"
	),
	
	
	-- Triangle mesh generation
	fn generateTriangle obj = (
		
		col_layer = getLayerByName layer_colliders
		cloned_mesh = copy obj
		cloned_mesh.rotation = obj.rotation
		cloned_mesh.pos = obj.pos
		cloned_mesh.name = "collider_" + obj.name
		cloned_mesh.parent = obj
		
		col_layer.addnode cloned_mesh
		addComponent obj "collider"
	),
	
	-- Create collider of a given type for the object passed 
	fn generateType obj type = (
		
		case type of
		(
			1: generateBasic obj
			2: generateConvex obj
			3: generateTriangle obj
		)
	)
)