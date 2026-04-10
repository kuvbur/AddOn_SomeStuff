* `Sync_flag`- a flag property that allows you to disable synchronization for this element. The property type is a boolean. The value can be calculated using a formula.

* _v1.72_ `Sync_class_flag` - flag for enabling classification processing. The other functions continue to work. If not found, the property with `Sync_flag` is used

* _v1.72_ `Sync_correct_flag` - flag to disable checking the accuracy of coordinates. The other functions continue to work. If not found, the property with `Sync_flag` is used

## Synchronization within the properties of a single object
`Sync_from' - reads values FROM another location and writes them to the current property. `Sync_to' - writes the value of the current property to another location (property, object parameter, project info).
* `Sync_from{PARAMETER NAME}` - copying the value of the GDL parameter to the property (property name in Latin, look in the GDL editor)
* `Sync_from{PARAMETER NAME; IGNORED VALUES}` - the same, but with the ability to set values that do not need to be recorded
* `Sync_from{description:PARAMETER DESCRIPTION_}` - copying the value of the GDL parameter to the property (the display name of the GDL parameter may be Cyrillic)
* `Sync_from{description:PARAMETER DESCRIPTION_; IGNORED VALUES}` - the same, but with the ability to set values that do not need to be recorded
* `Sync_from{Property:GROUP_NAME/PROPERTY_NAME}` - copying the value of another parameter
* `Sync_from{Property:GROUP_NAME/PROPERTY_NAME; IGNORED VALUES}` - the same, but with the ability to set values that do not need to be recorded
* `Sync_from{IFC:PROPERTY NAME}` - copies the value of the IFC property
* `Sync_to{id}` - copies the property value to the element ID. _ Does not work in AC22_
* _v1.6_ `Sync_to{PARAMETER NAME}` - copies the property value to the GDL parameter of the element
### _v1.69_ Array processing (read-only)
* `Sync_from{PARAMETER NAME; uniq}` - Output of unique array values (for text properties) or sums (for numbers)
* `Sync_from{PARAMETER NAME; sum}` - Output of concatenated array values (for text properties) or sums (for numbers)
* `Sync_from{PARAMETER NAME; max}` - The maximum value of the array
* `Sync_from{PARAMETER NAME; min}` - The minimum value of the array
If necessary, specify the range of reading arrays - specify first the values for rows, then for columns, in the format `(FIRST_STRING, LAST ROW)(FIRST COLUMN, LAST COLUMN)`. For example, the entry `Sync_from{PARAMETER NAME; unic(1,3)}` outputs unique values that range from the first to the third line inclusive. You can also use GDL object parameters when setting the range, for example `Sync_from{PARAMETER NAME; unic(1,nrow)}`
### _v1.6_ Coordinate processing
Handles the following types - Window, Door, Wall, Beam, Column, Object, Zone
For curtain wall panels, returns the center of the panel (x, y, z), for a column or object, returns the center of the column (x, y), and otm. bottom (z), for the zone - the center of the zone (x, y, without a mark, z is always 0)
* `Sync_from{symb_pos_x}`, `Sync_from{symb_pos_y}`, `Sync_from{symb_pos_z}` - coordinates of the element (for walls and beams - the starting point)
* `Sync_from{Coord:symb_pos_x}`, `Sync_from{Coord:symb_pos_y}`, `Sync_from{Coord:symb_pos_z}` - coordinates of the element (for walls and beams - the starting point)
* `Sync_from{Coord:symb_pos_sx}`, `Sync_from{Coord:symb_pos_sy}` - coordinates of the beginning of the element (for walls and beams)
* `Sync_from{Coord:symb_pos_ex}`, `Sync_from{Coord:symb_pos_ey}` - coordinates of the end of the element (for walls and beams)
* `Sync_from{Coord:symb_rotangle}` - output the rotation angle
* `Sync_from{Coord:symb_rotangle_mod5}`, `Sync_from{Coord:symb_rotangle_mod10}`, `Sync_from{Coord:symb_rotangle_mod45}`, `Sync_from{Coord:symb_rotangle_mod90}`, `Sync_from{Coord:symb_rotangle_mod180}` is the result of an integer angle divisions
* `Sync_from{Coord:north_dir}` - the angle between the element and the north of the project. Outputs -1 for objects and zones.
* _v1.69_ `Sync_from{Coord:north_dir_str}` - Orientation to the cardinal directions, for walls and openings. The data type is a string. Outputs a blank line for objects and zones.
* _v1.70_ `Sync_from{Coord:north_dir_eng}` - Orientation to the cardinal directions (in English), for walls and openings. The data type is a string. Outputs an empty string for objects and zones.
* _v1.72_ `Sync_to{Coord:symb_pos_x}`, `Sync_to{Coord:symb_pos_y}`, `Sync_to{Coord:symb_pos_z}` - setting the coordinates of an element (objects, columns)
* _v1.72_ `Sync_to{Coord:symb_pos_sx}`, `Sync_to{Coord:symb_pos_sy}` - setting the coordinate of the beginning of the element (for walls and beams)
* _v1.72_ `Sync_to{Coord:symb_pos_ex}`, `Sync_to{Coord:symb_pos_ey}` - setting the coordinate of the end of the element (for walls and beams)
* _v1.72_ `Sync_to{Coord:symb_rotangle}` - setting the rotation angle
**Checking for the presence of a fractional part in coordinates/angle**
The function returns TRUE if the fractional part is missing and FALSE if the fractional part is greater than the tolerance.
The tolerances are 0.001 mm for coordinates and length, 0.00001 degrees for angles.
There are two ways to disable coordinate verification - by disabling synchronization completely (setting the value of the flag property to FALSE), or create a Boolean property with the description `Sync_correct_flag` and set it to FALSE. In this case, only the reconciliation of the presence of a fractional part for this element will be disabled, the rest of the synchronization will work.
* `Sync_from{Coord:symb_pos_correct}` - general X and Y check for start and end
* `Sync_from{Coord:symb_pos_x_correct}`, `Sync_from{Coord:symb_pos_y_correct}`
* `Sync_from{Coord:symb_pos_sx_correct}`, `Sync_from{Coord:symb_pos_sy_correct}` - checking the beginning of the element (for walls and beams)
* `Sync_from{Coord:symb_pos_ex_correct}`, `Sync_from{Coord:symb_pos_ey_correct}` - checking the end of the element (for walls and beams)
* `Sync_from{Coord:symb_rotangle_fraction}` - fractional part of the rotation angle 
* `Sync_from{Coord:symb_rotangle_correct}` - the presence of a fractional part of the rotation angle of the element with a tolerance of 0.00001
* `Sync_from{Coord:symb_rotangle_correct_1000}` - presence of a fractional part of the rotation angle of the element with a tolerance of 0.001
### _v1.6_ Entry in the properties of the project location data
* `Sync_from{Glob:GLOB_NORTH_DIR}`
* `Sync_from{Glob:GLOB_PROJECT_LONGITUDE}`
* `Sync_from{Glob:GLOB_PROJECT_LATITUDE}`
* `Sync_from{Glob:GLOB_PROJECT_ALTITUDE}`
* `Sync_from{Glob:GLOB_SUN_AZIMUTH}`
* `Sync_from{Glob:GLOB_SUN_ALTITUDE}`
### _v1.6_ Entry in the project information
*` Sync_to{Info:PROPERTY NAME}` - Record the value of the property in the user property of the project information. It only works when all items are synchronized, or when selected items are synchronized.
### _v1.6_ Processing of morph lines
* `Sync_from{Morph:L}` - the total length of the morph line
* `Sync_from{Morph:Lx}`, `Sync_from{Morph:Ly}`, `Sync_from{Morph:Lz}` - length along the corresponding axis
* `Sync_from{Morph:Max_x}`, `Sync_from{Morph:Max_y}`, `Sync_from{Morph:Max_z}` - maximum coordinate
* `Sync_from{Morph:Min_x}`, `Sync_from{Morph:Min_y}`, `Sync_from{Morph:Min_z}` - minimum coordinate
* `Sync_from{Morph:A}`, `Sync_from{Morph:B}`, `Sync_from{Morph:ZZYZX}` - similar to the size of the bibl. element, can be used to output overall dimensions
### [Composition processing конструкций](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%92%D1%8B%D0%B2%D0%BE%D0%B4-%D1%81%D0%BE%D1%81%D1%82%D0%B0%D0%B2%D0%B0-%D0%BA%D0%BE%D0%BD%D1%81%D1%82%D1%80%D1%83%D0%BA%D1%86%D0%B8%D0%B8)
* `Sync_from{Material:Layers; " PATTERN_LINE "}`
* _v1.6_ `Sync_from{Material:Layers,NUMBER_THE PEN; " PATTERN_LINE "}` - to extract information about the composition of the multilayer profile at the intersection with the line with the specified pen number. The order of the layers is determined by the location of the circle.
* _v1.70_ `Sync_from{Material:Layers_inv; " PATTERN_LINE "}` - inversion of the layer order
* _v1.70_ `Sync_from{Material:Layers_auto; " TEMPLATE STRING "}` - auto-inversion of the layer order (layers are displayed in the same order as they are displayed when previewing the cross section)
* _v1.72_ `Sync_from{Material:Layers; " STRING_ PATTERN < FORMULA > "}` - with the calculation of the expression inside the characters `<` `>`
* _v1.72_ For custom properties of building material, add `{@property' to the description:buildingmaterialproperties}`
### _v1.72_ Changing the element layer
* `Sync_to{Attribute:Layer}` - the search is performed by the full name of the layer (with the extension) 
### _v1.72_ [Classification элемента](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%90%D0%B2%D1%82%D0%BE%D0%BC%D0%B0%D1%82%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%BE%D0%B5-%D0%BA%D0%BB%D0%B0%D1%81%D1%81%D0%B8%D1%84%D0%B8%D1%86%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%BE%D0%B2,-%D0%B2%D1%8B%D0%B2%D0%BE%D0%B4-%D0%B8%D0%BD%D1%84%D0%BE%D1%80%D0%BC%D0%B0%D1%86%D0%B8%D0%B8-%D0%BE-%D0%BA%D0%BB%D0%B0%D1%81%D1%81%D0%B5)
* Enabling classification for unclassified elements - add `some_stuff_class` to the class description
* `Sync_to{Class:CLASSIFICATION_NAME}` A text property. The search is performed by the class Id.
* `Sync_from{Class:CLASSIFICATION_NAME; FullName}` - outputs the full name of the class
## Synchronization of properties of hierarchical structures (curtain wall, elements in the zone)
Similarly to synchronization within a single element, with the addition of the _sub suffix, for example Sync_from_sub{Property:GROUP_NAME/PROPERTY_NAME}, written in the curtain wall will read the property from the nested element (panel, frame, accessory). Sync_to_sub works similarly{Property:GROUP_NAME/PROPERTY_NAME}
## Reset the property value to the default value
* `Sync_reset` - Resets the property everywhere - in the placed elements, in favorites, on layouts - everywhere. Synchronization of the resettable property is disabled while Sync_reset _ is in the description, it does not work in the AC27 version_ 
## [Dimension Rounding](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%A4%D1%83%D0%BD%D0%BA%D1%86%D0%B8%D0%B8-%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D1%8B-%D1%81-%D1%80%D0%B0%D0%B7%D0%BC%D0%B5%D1%80%D0%B0%D0%BC%D0%B8)
