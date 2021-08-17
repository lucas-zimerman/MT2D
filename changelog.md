# Changelog

## Unreleased

### Fixes

### Features

- Add common object parameters as variables (Aceleration, Position, Size)
- Add functions Object_Add_Variable and Object_Add_Variables

### Breaking

- User_Vars is always initialized and you should add additional variables using Object_Add_Variable or Object_Add_Variables.
- If you replace the content of User_Vars, ObjectCore will not work when dealing with primary variables.
- To Capture the primitive parameters you can use one of the following methods:
method1:
///c
 //Set 12 to the posX coord
 *(int*)Object.User_Vars[Object_PosX_Var_Index].Data = 12;
 ///
 method2: 
 ///c
ObjectPosX(Object) = 12;