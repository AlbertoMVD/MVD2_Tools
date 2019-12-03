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

-- Class used to export the meshes as .obj or binary files
struct MeshManager 
(
	--fs = MVDJson(),           
	
	-- Method to export mesh in .obj format
	fn exportItem obj epath = 
	(
		-- Build the path and export file
		select obj
		local full_path = (epath + obj.name + ".obj")
		exportFile full_path #noPrompt selectedOnly:true #noPrompt  
		return full_path
	),
	
	-- Method to export a mesh in binary format
	fn exportItemBinary obj epath = (
		
		/* TO-DO
		(
			-- Export the given object in our custom format!
			-- Class 7
		)
		*/
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