
function createRandomTerrain size density = (
	
	myplane = Plane width:size length:size lengthsegs:density widthsegs:density wirecolor:brown
	myplane = convertToPoly(myplane)
	
	addmodifier myplane (NoiseModifier())
	myplane.modifiers[#noise].scale = size * 0.01
	myplane.modifiers[#noise].strength = [0,0,4.1]
	myplane.modifiers[#noise].seed = (random 1 1000)
	return myplane
)

delete $*
global size = 100
global density = 20

createRandomTerrain size density