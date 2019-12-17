global size = 100
global density = 20
global randomTerrain = undefined 
global randomRoad = undefined 
delete $*

-- We create the random terrain given the parameters
function createRandomTerrain size density = (
	
	myplane = Plane width:size length:size lengthsegs:density widthsegs:density wirecolor:brown
	myplane = convertToPoly(myplane)
	
	addmodifier myplane (NoiseModifier())
	myplane.modifiers[#noise].scale = size * 0.01
	myplane.modifiers[#noise].strength = [0,0,4.1]
	myplane.modifiers[#noise].seed = (random 1 1000)
		
	return myplane
)

-- Define the spline for the road
function drawSpline knots size density = (
	
	-- We want to create an spline
	-- With initial and end points provided.
	ss = SplineShape pos:[0,0,0] render_renderable:true render_displayRenderMesh:true render_rectangular:true render_viewport_rectangular:true
	ss.render_length = 1
	ss.render_width = 4
	addNewSpline ss
	
	for i in knots do (
		addKnot ss 1 #smooth #curve i
	)

	updateShape ss
	minimumDistance = (size / density) * 0.5
	return conformRoad ss minimumDistance
)

function conformRoad road minDistance = (
	
	ss = SplineShape pos:[0,0,0] render_renderable:true render_displayRenderMesh:true render_rectangular:true render_viewport_rectangular:true
	ss.render_length = 1
	ss.render_width = 4
	addNewSpline ss
	
	lengthTotal = curveLength road
	lengthStep = (lengthTotal / minDistance) as Integer
	
	for i = 1 to lengthStep do (
		
		-- Interpolate along the spline "road"
		-- get a position on each step 'i'
		-- Raycast to the terrain and get the raycasted hit positon
		
		-- add the knot to the set
		addKnot ss 1 #smooth #curve pos
	)
	
	updateShape ss
	ss.name = "TheRoad"
	
	delete road
	return ss
)

-- Determine initial knots of the spline and retrieve general information
function createRandomRoad size density = (
	
	totalFaces = polyop.getNumFaces randomTerrain
	step = totalfaces * 0.25

	marginStep = (size + (random 2 (size - 2)))
	pStart 	= polyop.getFaceCenter randomTerrain 1
	pEnd 	= polyop.getFaceCenter randomTerrain totalFaces
	pMid1 	= polyop.getFaceCenter randomTerrain (random marginStep step)
	pMid2 	= polyop.getFaceCenter randomTerrain (random (3*step) (totalFaces-marginStep))
	
	drawSpline #(pStart, pMid1, pMid2, pEnd) size density
)

randomTerrain = createRandomTerrain size density
randomRoad = createRandomRoad size density
