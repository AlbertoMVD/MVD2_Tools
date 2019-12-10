/* 
Component are added as attribute to nodes
This attributes have properties and a rollout to edit them
Components are persistent 
*/

-- Component collider, used to define collider object properties
component_collider = attributes component_collider
(
	-- Collider parameters
	parameters cmp_col rollout:cmp_col
	(
		col_group type: #string ui:col_group default:"All"
		col_mask type: #string ui:col_mask default:""
		is_trigger type: #boolean ui:trigger default:false
		is_dynamic type: #boolean ui:dynamic default:false
		is_controller type: #boolean ui:controller default:false
		is_gravity type: #boolean ui:isgravity default:false
	)
	
	-- Rollout UI of a collider
	rollout cmp_col "Component Collider" width:162 height:174
	(
		edittext 'col_group' "Group" width:130 height:20 align:#center
		edittext 'col_mask' "Mask" width:130 height:20 align:#center
		checkbox 'trigger' "Is Trigger" width:70 height:15 type:#BOOLEAN align:#center
		checkbox 'dynamic' "Is Dynamic" width:76 height:15 type:#BOOLEAN align:#center
		checkbox 'controller' "Is Controller" width:83 height:15 type:#BOOLEAN align:#center
		checkbox 'isgravity' "Is Gravity" width:71 height:15 type:#BOOLEAN align:#center
		
	)
)


-- Methods to create/destroy components

-- Get a component by it's type (string)
-- Add here new components in the future
function getByType type = (

	if type == "collider" then ( return component_collider )
	if type == "tag" then ( return component_tag )
	if type == "prefab" then ( return component_prefab )
	if type == "technique" then ( return component_technique )
	if type == "audio" then ( return component_audio )
	if type == "particles" then ( return component_particles )
	if type == "rotator" then ( return component_rotator )
	if type == "neon" then ( return component_neon )
)

-- Function to add a component to an object
function addComponent c_selection type = (

	local c_type = getByType type
	
	for obj in c_selection do (
		custAttributes.add obj c_type
	)
)

-- Remove component from object
function removeComponent c_selection type = (
	
	local c_type = getByType type
	
	for obj in c_selection do (
		
		local comp = retrieveDefComponent obj c_type
		if comp != undefined then ( custAttributes.delete obj comp)
	
	)
)

-- Obtain the definition string from a component type
function retrieveDefComponent obj type = (
	
	local totalIndex = custAttributes.count obj
	
	for att_index = 1 to totalIndex do (
		local attr_iter = custAttributes.getdef obj att_index
		if attr_iter.name == type.name then ( return attr_iter )
	)
	
	return undefined
)

-- Get a component given a type
function retrieveComponent obj type = (
	
	local totalIndex = custAttributes.count obj
	
	for att_index = 1 to totalIndex do (
		local attr_iter = custAttributes.getdef obj att_index
		if attr_iter.name == type.name then ( 
			local comp_final = custAttributes.get obj attr_iter
			return comp_final
		)
	)
	
	return undefined
)