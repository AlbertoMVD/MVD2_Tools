-- Remove macro function to avoid stacking up macros on toolbar
function removeMenu m_name = 
(
	del_menu = menuMan.findMenu m_name
	if del_menu != undefined do
		menuMan.unregistermenu del_menu
)

macroScript LaunchMenu category:"MVD" --macroscript menu
(
	-- Launch the menu UI
)

--Adding menu macro process with menuMan
removeMenu "MVD Tools"
theMainMenu = menuMan.getMainMenuBar() --get the main menu bar
theMenu = menuMan.createMenu "MVD Tools" --create a menu called Forum Help
theSubMenu = menuMan.createSubMenuItem "Launch tool" theMenu --create a SubMenuItem
theMainMenu.addItem theSubMenu (theMainMenu.numItems()+1) --add the SubMenu to the Main Menu
theAction = menuMan.createActionItem "LaunchMenu" "MVD" --create an ActionItem from the MacroScript
theMenu.addItem theAction (theMenu.numItems()+1) --add the ActionItem to the menu
menuMan.updateMenuBar() --update the menu bar