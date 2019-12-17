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
	newRoad = conformRoad ss minimumDistance
	return newRoad
)

function conformRoad road minDistance = (
	
	ss = SplineShape pos:[0,0,0] render_renderable:true render_displayRenderMesh:true render_rectangular:true render_viewport_rectangular:true
	ss.render_length = 1
	ss.render_width = 4
	addNewSpline ss
	
	lengthTotal = curveLength road
	stepsTotal = (lengthTotal / minDistance) as Integer
	
	for i = 1 to stepsTotal do (
		
		-- Interpolate along the spline "road"
		-- get a position on each step 'i'
		currentInterp = ((i as float) / (stepsTotal as float))
		pos = lengthInterp road currentInterp
		
		-- Raycast to the terrain and get the raycasted hit positon
		ray_dir = Ray (pos + [0,0,1000]) [0,0,-1]
		ray_int = intersectRay randomTerrain ray_dir
		
		-- In case we have an intersection, then change the position of the knot
		-- We are not handling other situations were there is no collision
		if ray_int != undefined then (
			pos = ray_int.pos
		)
		
		-- add the knot to the set
		addKnot ss 1 #smooth #curve pos
	)
	
	updateShape ss
	ss.name = "TheRoad"
	
	delete road
	return ss
)

-- Create trees along spline (not colliding between them or with the road) (ex c)
function createRandomTrees amount density = (
	
	-- Loop through random road
	-- Interpolate through it given the density as step
	-- Places trees along the spline given the amount provided
	
)


-- Create the car animation and camera animation (d & e)
function createAnimationCar spent = (
	
	-- Create a car or use -- ImportFile "audi_car.obj" #noPrompt
	-- create a camera to follow the car
	
	-- Place the car and the camera on road start
	-- Attach the camera to the car with proper orientation
	
	animate on for t = 0 to spent by 5 
	do at time t
	(
		-- Interpolate along the road spline
		-- Set a keyframe of my car position at given time
		-- Set a keyframe of my car rotation at given time
		
		-- lengthtangent to get the tangent direction
		-- we can get the angle with acos (dot val1 val2)
		
		-- Camera must follow the camera
	)
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
randomTrees = createRandomTrees size density
randomAnimation = createAnimationCar 320