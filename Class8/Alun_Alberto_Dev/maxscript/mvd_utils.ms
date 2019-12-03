-- Utility functions declared here
-- Any extra functionality needed to be provided within this context.

-- Check if folder exists
fn doesFolderExist f create:false = (
	
	local val = if (doesFileExist f) and (getfileattribute f #directory) then true else false
	if not val and create then
	(
		val = makeDir f
	)
	
	return val
) 

-- Get the current directory
fn getCurrentAssetDir = (
	
	local folder = (pathConfig.removePathLeaf (getThisScriptFilename()) )
	return (getfilenamepath (trimRight folder "\\/"))
)

-- Get the layer from layermanager given a name
fn getLayerByName lname = (
	
	for i = 0 to layerManager.count-1 do(
		
		local ilayer = layerManager.getLayer i
		if ilayer.name == lname do ( return ilayer )
	)	
	
	-- create a new layer if it doesn't exist
	layer = LayerManager.newLayer()
	layer.setname lname
	
	return layer
)

-- Get all the materials that belong to a mesh
fn getMaterialsUsedByMesh obj = (
	
	-- Get faces by material id
	local num_faces = getNumFaces obj
	local faces_by_mat = #()
	for face_id = 1 to num_faces do (
		local mat_id = getFaceMatID obj face_id
		if faces_by_mat[ mat_id ] == undefined then (
			faces_by_mat[ mat_id ] = #()
		)			
		append faces_by_mat[ mat_id ] face_id
	)
	
	return faces_by_mat
)

-- Write a chunk of bytes into stream
fn writeChunk fs id nbytes = (
	
	WriteLong fs id #unsigned
	WriteLong fs nbytes #unsigned
)