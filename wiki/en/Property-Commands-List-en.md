# List of All Available Commands

## Synchronization Control Flags

| Command             | Description                                                                                                                         |
| ------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| `Sync_flag`         | Property flag for disabling element synchronization.<br>**Type:** Matching criterion. Can be calculated by a formula.              |
| `Sync_class_flag`   | Flag for enabling classification processing (_v1.72+_).<br>If not found, `Sync_flag` is used. Other functions remain operational.      |
| `Sync_correct_flag` | Flag for disabling coordinate accuracy verification (_v1.72+_).<br>If not found, `Sync_flag` is used. Other functions remain operational. |

## Synchronization within Properties of a Single Object

`Sync_from` - reads values FROM another location and writes them TO the current property. `Sync_to` - writes the value of the current property to another location (property, object parameter, project info).

| Command                                                            | Description                                                                 |
| ------------------------------------------------------------------ | --------------------------------------------------------------------------- |
| `Sync_from`                                                        | Reads values from the source and writes them to the current property        |
| `Sync_to`                                                          | Writes the value of the current property to the target location             |
| **Reading Data (`Sync_from`)**                                     |                                                                             |
| `Sync_from{PARAMETER_NAME}`                                        | Copies the value of a GDL parameter to the property (parameter name in Latin characters) |
| `Sync_from{PARAMETER_NAME; IGNORED_VALUES}`                        | Copies the value of a GDL parameter, excluding specified values             |
| `Sync_from{description:PARAMETER_DESCRIPTION}`                     | Copies the value based on the displayed parameter name (supports Cyrillic)  |
| `Sync_from{description:PARAMETER_DESCRIPTION; IGNORED_VALUES}`     | Copies the value based on the displayed name, excluding specified values    |
| `Sync_from{Property:GROUP/PROPERTY}`                               | Copies the value from another property                                      |
| `Sync_from{Property:GROUP/PROPERTY; IGNORED_VALUES}`               | Copies the value from another property, excluding specified values          |
| `Sync_from{IFC:PROPERTY_NAME}`                                     | Copies the value of an IFC property                                         |
| **Writing Data (`Sync_to`)**                                       |                                                                             |
| `Sync_to{id}`                                                      | Writes the property value to the Element ID (does not work in AC22)         |
| `Sync_to{PARAMETER_NAME}`                                          | Writes the property value to the GDL parameter of the element (_v1.6+_)     |

## Ignored Values (_v1.77+_)

- `empty` - Ignore empty (for strings) and zero (for numbers) values
- `trim_empty` - Ignore values consisting only of spaces (for strings) and zero (for numbers)
- `def` - Reset the property to its default value if no suitable values for writing are found

### Processing GDL Object Arrays (Read-Only) _v1.69+_

| Command                          | Description                                                                         |
| -------------------------------- | -------------------------------------------------------------------------------- |
| `Sync_from{ИМЯ_ПАРАМЕТРА; uniq}` | Returns unique values from the array (for text) or the sum (for numbers)        |
| `Sync_from{ИМЯ_ПАРАМЕТРА; sum}`  | Returns concatenated values from the array (for text) or the sum (for numbers) |
| `Sync_from{ИМЯ_ПАРАМЕТРА; max}`  | Returns the maximum value from the array                                         |
| `Sync_from{ИМЯ_ПАРАМЕТРА; min}`  | Returns the minimum value from the array                                          |

If it is necessary to specify a reading range for arrays, the row values are specified first, followed by the column values, in the format `(FIRST_ROW, LAST_ROW)(FIRST_COLUMN, LAST_COLUMN)`. For example, the entry `Sync_from{ИМЯ_ПАРАМЕТРА; uniq(1,3)}` will output unique values located within the range from the first to the third row inclusive. GDL object parameters can also be used when defining the range, for example `Sync_from{ИМЯ_ПАРАМЕТРА; uniq(1,nrow)}`

### _v1.6_ Coordinate Processing

Processes the following element types: Window, Door, Wall, Beam, Column, Object, Zone.
For curtain wall panels, returns the panel center (x, y, z); for columns or objects - the column/object center (x, y) and bottom elevation (z); for zones - the zone center (x, y, no elevation, z is always 0).

| Command                                                                                              | Description                                                                                                                                                                                                                                                                                                                                       |
| ---------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Read Coordinates**                                                                                 |                                                                                                                                                                                                                                                                                                                                                |
| `Sync_from{symb_pos_x}`, `Sync_from{symb_pos_y}`, `Sync_from{symb_pos_z}`                            | Base coordinates of the element (for walls/beams - the start point)                                                                                                                                                                                                                                                                                    |
| `Sync_from{Coord:symb_pos_sx}`, `Sync_from{Coord:symb_pos_sy}`                                       | Start coordinates of the element (for walls/beams)                                                                                                                                                                                                                                                                                                    |
| `Sync_from{Coord:symb_pos_ex}`, `Sync_from{Coord:symb_pos_ey}`                                       | End coordinates of the element (for walls/beams)                                                                                                                                                                                                                                                                                                     |
| `Sync_from{Coord:symb_pos_sx}`, `Sync_from{Coord:symb_pos_sy}`                                       | Absolute coordinates of the opening center _<br>(v1.74, windows/doors)_                                                                                                                                                                                                                                                                                  |
| `Sync_from{Coord:symb_pos_lo_x}`, `Sync_from{Coord:symb_pos_lo_y}`, `Sync_from{Coord:symb_pos_lo_z}` | Coordinates relative to the user-defined origin _<br>(v1.73+)_                                                                                                                                                                                                                                                                                |
| `Sync_from{Coord:symb_pos_lo_sx}`, `Sync_from{Coord:symb_pos_lo_sy}`                                 | Element start relative to the user-defined origin _<br>(v1.73+, walls/beams)_                                                                                                                                                                                                                                                              |
| `Sync_from{Coord:symb_pos_lo_ex}`, `Sync_from{Coord:symb_pos_lo_ey}`                                 | Element end relative to the user-defined origin _<br>(v1.73+, walls/beams)_                                                                                                                                                                                                                                                               |
| `Sync_from{Coord:symb_pos_lo_sx}`, `Sync_from{Coord:symb_pos_lo_sy}`                                 | Opening center coordinates relative to the user-defined origin _<br>(v1.74+)_                                                                                                                                                                                                                                                                  |
| `Sync_from{Coord:symb_pos_sp_x}`, `Sync_from{Coord:symb_pos_sp_y}`, `Sync_from{Coord:symb_pos_sp_z}` | Coordinates relative to the project location point (SurveyPoint) _<br>(v1.77+)_                                                                                                                                                                                                                                                              |
| `Sync_from{Coord:symb_pos_sp_sx}`, `Sync_from{Coord:symb_pos_sp_sy}`                                 | Element start relative to the project location point _<br>(v1.77+, walls/beams)_                                                                                                                                                                                                                                                          |
| `Sync_from{Coord:symb_pos_sp_ex}`, `Sync_from{Coord:symb_pos_sp_ey}`                                 | Element end relative to the project location point _<br>(v1.77+, walls/beams)_                                                                                                                                                                                                                                                           |
| **Angles and Orientation**                                                                                |                                                                                                                                                                                                                                                                                                                                                |
| `Sync_from{Coord:symb_rotangle}`                                                                     | Rotation angle of the element (for columns: sum of axis and slant angles)                                                                                                                                                                                                                                                                                       |
| `Sync_from{Coord:symb_rotangle_slant}`                                                               | Slant rotation angle of a column _<br>(v1.74+)_                                                                                                                                                                                                                                                                                                   |
| `Sync_from{Coord:symb_rotangle_axis}`                                                                | Cross-section rotation angle of a column _<br>(v1.74+)_                                                                                                                                                                                                                                                                                                   |
| `Sync_from{Coord:symb_rotangle_mod...}`                                                              | Integer division of the angle (mod5, mod10, etc.)                                                                                                                                                                                                                                                                                                |
| `Sync_from{Coord:north_dir}`                                                                         | Angle between the element and project north                                                                                                                                                                                                                                                                                                         |
| `Sync_from{Coord:north_dir_str}`                                                                     | Cardinal orientation (string) _<br>(v1.69+)_                                                                                                                                                                                                                                                                                           |
| `Sync_from{Coord:north_dir_eng}`                                                                     | Cardinal orientation (English) _<br>(v1.70+)_                                                                                                                                                                                                                                                                                            |
| **Write Coordinates**                                                                                 |                                                                                                                                                                                                                                                                                                                                                |
| `Sync_to{Coord:symb_pos_x}`, `Sync_to{Coord:symb_pos_y}`, `Sync_to{Coord:symb_pos_z}`                | Write element coordinates _<br>(v1.72+, objects/columns)_                                                                                                                                                                                                                                                                                      |
| `Sync_to{Coord:symb_pos_sx}`, `Sync_to{Coord:symb_pos_sy}`                                           | Write start coordinates _<br>(v1.72+, walls/beams)_                                                                                                                                                                                                                                                                                            |
| `Sync_to{Coord:symb_pos_ex}`, `Sync_to{Coord:symb_pos_ey}`                                           | Write end coordinates _<br>(v1.72+, walls/beams)_                                                                                                                                                                                                                                                                                             |
| `Sync_to{Coord:symb_rotangle}`                                                                       | Write rotation angle _<br>(v1.72)_                                                                                                                                                                                                                                                                                                             |
| **Special Functions**                                                                              |                                                                                                                                                                                                                                                                                                                                                |
| `Sync_from{Coord:locorigin_x}`, `Sync_from{Coord:locorigin_y}`, `Sync_from{Coord:locorigin_z}`       | User-defined origin coordinates _<br>(v1.73)_                                                                                                                                                                                                                                                                                              |
| `Sync_from{Coord:windoor_in_wall}`                                                                   | Check if an opening is placed in a wall _<br>(v1.74+, False = not in a wall)_                                                                                                                                                                                                                                                                            |
| `Sync_from{Coord:locOrigin_x}`, `Sync_from{Coord:locOrigin_y}`, `Sync_from{Coord:locOrigin_z}`       | User-defined origin coordinates                                                                                                                                                                                                                                                                                                            |
| `Sync_from{Coord:offsetOrigin_x}`, `Sync_from{Coord:offsetOrigin_y}`                                 | This function returns the offset of the virtual coordinate system origin relative to the world coordinate system origin. The offset is set when the file is opened if the model's "weight point" is sufficiently far from the world origin. Once set, all coordinates obtained via the API are measured from this virtual origin. |

### Checking for Fractional Parts in Coordinates/Angles

[YouTube video example](https://youtu.be/a9Dnvo1P-rk)
[Rutube video example](https://rutube.ru/video/c853151ae9a8b3582bd3a10fb52863f9)

The functions return TRUE if the fractional part is absent and FALSE if the fractional part exceeds the tolerance.

Tolerances are 0.001 mm for coordinates and length, and 0.00001 degrees for angles.

Coordinate checking can be disabled in two ways: by disabling synchronization completely (by setting the flag property value to FALSE), or by creating a boolean property with the description `Sync_correct_flag` and setting it to FALSE. In this case, only the fractional part check for this specific element will be disabled; the rest of the synchronization will continue to work.

| Command                                                                                        | Description                                                                                                                                         |
| ---------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Basic Checks**                                                                           |                                                                                                                                                  |
| `Sync_from{Coord:symb_pos_correct}`                                                            | General check of start/end points in X/Y (returns FALSE if a fractional part exists in one of the coordinates)                                                    |
| `Sync_from{Coord:symb_pos_x_correct}`, `Sync_from{Coord:symb_pos_y_correct}`                   | X/Y coordinate check                                                                                                                           |
| `Sync_from{Coord:symb_pos_sx_correct}`, `Sync_from{Coord:symb_pos_sy_correct}`                 | Element start point check (wall/beam)                                                                                                           |
| `Sync_from{Coord:symb_pos_ex_correct}`, `Sync_from{Coord:symb_pos_ey_correct}`                 | Element end point check (wall/beam)                                                                                                            |
| `Sync_from{Coord:symb_rotangle_fraction}`                                                      | Rotation angle fractional part                                                                                                                      |
| `Sync_from{Coord:symb_rotangle_correct}`                                                       | Rotation angle fractional part check (tolerance 0.00001°)                                                                                                    |
| `Sync_from{Coord:symb_rotangle_correct_1000}`                                                  | Rotation angle fractional part check (tolerance 0.001°)                                                                                                      |
| `Sync_from{Coord:l_correct}`                                                                   | Element length check                                                                                                                          |
| **Strict Checks (tolerance 0.000001 mm)**                                                      |                                                                                                                                                  |
| `Sync_from{Coord:symb_pos_correct_hard}`                                                       | General check of start/end points (returns FALSE if a fractional part exists in one of the coordinates)                                                           |
| `Sync_from{Coord:symb_pos_x_correct_hard}`, `Sync_from{Coord:symb_pos_y_correct_hard}`         | Strict X/Y check                                                                                                                             |
| `Sync_from{Coord:symb_pos_sx_correct_hard}`, `Sync_from{Coord:symb_pos_sy_correct_hard}`       | Strict start point check                                                                                                                          |
| `Sync_from{Coord:symb_pos_ex_correct_hard}`, `Sync_from{Coord:symb_pos_ey_correct_hard}`       | Strict end point check                                                                                                                           |
| `Sync_from{Coord:l_correct_hard}`                                                              | Strict element length check                                                                                                                  |
| **Checks Relative to User Origin**                                             |                                                                                                                                                  |
| `Sync_from{Coord:symb_pos_lo_correct}`                                                         | General check of start/end points (returns FALSE if a fractional part exists in one of the coordinates) _<br>(v1.73+)_                                            |
| `Sync_from{Coord:symb_pos_lo_x_correct}`, `Sync_from{Coord:symb_pos_lo_y_correct}`             | X/Y check _<br>(v1.73+)_                                                                                                                      |
| `Sync_from{Coord:symb_pos_lo_sx_correct}`, `Sync_from{Coord:symb_pos_lo_sy_correct}`           | Start point check _<br>(v1.73+, walls/beams)_                                                                                                      |
| `Sync_from{Coord:symb_pos_lo_ex_correct}`, `Sync_from{Coord:symb_pos_lo_ey_correct}`           | End point check _<br>(v1.73+, walls/beams)_                                                                                                       |
| `Sync_from{Coord:symb_pos_lo_correct_hard}`                                                    | Strict general check _<br>(v1.73)_                                                                                                             |
| `Sync_from{Coord:symb_pos_lo_x_correct_hard}`, `Sync_from{Coord:symb_pos_lo_y_correct_hard}`   | Strict X/Y check _<br>(v1.73+)_                                                                                                              |
| `Sync_from{Coord:symb_pos_lo_sx_correct_hard}`, `Sync_from{Coord:symb_pos_lo_sy_correct_hard}` | Strict start point check _<br>(v1.73+, walls/beams)_                                                                                              |
| `Sync_from{Coord:symb_pos_lo_ex_correct_hard}`, `Sync_from{Coord:symb_pos_lo_ey_correct_hard}` | Strict end point check _<br>(v1.73+, walls/beams)_                                                                                               |
| **Checks Relative to Project Location**                                               |                                                                                                                                                  |
| `Sync_from{Coord:symb_pos_sp_correct}`                                                         | General check of start/end points (returns FALSE if a fractional part exists in one of the coordinates) _<br>(v1.77+)_                                            |
| `Sync_from{Coord:symb_pos_sp_x_correct}`, `Sync_from{Coord:symb_pos_sp_y_correct}`             | X/Y check _<br>(v1.77+)_                                                                                                                      |
| `Sync_from{Coord:symb_pos_sp_sx_correct}`, `Sync_from{Coord:symb_pos_sp_sy_correct}`           | Start point check _<br>(v1.77+, walls/beams)_                                                                                                      |
| `Sync_from{Coord:symb_pos_sp_ex_correct}`, `Sync_from{Coord:symb_pos_sp_ey_correct}`           | End point check _<br>(v1.77+, walls/beams)_                                                                                                       |
| `Sync_from{Coord:symb_pos_sp_correct_hard}`                                                    | Strict general check _<br>(v1.73)_                                                                                                             |
| `Sync_from{Coord:symb_pos_sp_x_correct_hard}`, `Sync_from{Coord:symb_pos_sp_y_correct_hard}`   | Strict X/Y check _<br>(v1.77+)_                                                                                                              |
| `Sync_from{Coord:symb_pos_sp_sx_correct_hard}`, `Sync_from{Coord:symb_pos_sp_sy_correct_hard}` | Strict start point check _<br>(v1.77+, walls/beams)_                                                                                              |
| `Sync_from{Coord:symb_pos_sp_ex_correct_hard}`, `Sync_from{Coord:symb_pos_sp_ey_correct_hard}` | Strict end point check _<br>(v1.77+, walls/beams)_                                                                                               |
| **Special Checks**                                                                       |                                                                                                                                                  |
| `Sync_from{Coord:has_distant_element}`                                                         | Checks for the presence of elements in the project located far from the coordinate origin. The distance is determined by ArchiCAD itself. _<br>(v1.76+)_ |

### _v1.6_ Writing Project Location Data to Properties

| Command                                                                                                                   | Description                                                                                                                       |
| ------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| `Sync_from{Glob:GLOB_NORTH_DIR}`                                                                                          | North direction in degrees                                                                                                        |
| `Sync_from{Glob:GLOB_PROJECT_LONGITUDE}`                                                                                  | Longitude in degrees                                                                                                              |
| `Sync_from{Glob:GLOB_PROJECT_LATITUDE}`                                                                                   | Latitude in degrees                                                                                                               |
| `Sync_from{Glob:GLOB_PROJECT_ALTITUDE}`                                                                                   | Altitude in meters                                                                                                                |
| `Sync_from{Glob:GLOB_SUN_AZIMUTH}`                                                                                        | Sun position component in degrees                                                                                                 |
| `Sync_from{Glob:GLOB_SUN_ALTITUDE}`                                                                                       | Sun position component in degrees                                                                                                 |
| `Sync_from{Glob:surveyPointPosition_x}`, `Sync_from{Glob:surveyPointPosition_y}`, `Sync_from{Glob:surveyPointPosition_z}` | Project location point coordinates _<br>(v1.77+)_                                                                                 |
| `Sync_from{Glob:eastings}`                                                                                                | Easting position in the target coordinate reference system _<br>(v1.77+)_                                                         |
| `Sync_from{Glob:northings}`                                                                                               | Northing position in the target coordinate reference system _<br>(v1.77+)_                                                         |
| `Sync_from{Glob:orthogonalHeight}`                                                                                        | Orthogonal height relative to the specified vertical datum _<br>(v1.77+)_                                                          |
| `Sync_from{Glob:xAxisAbscissa}`                                                                                           | Easting value of the endpoint of the vector defining the local x-axis of the engineering coordinate system _<br>(v1.77+)_          |
| `Sync_from{Glob:xAxisOrdinate}`                                                                                           | Northing value of the endpoint of the vector defining the local x-axis of the engineering coordinate system _<br>(v1.77+)_         |
| `Sync_from{Glob:scale}`                                                                                                   | Scale used if the CRS units differ from the engineering coordinate system units _<br>(v1.77+)_                                     |

### _v1.6_ Writing to Project Information

\*` Sync_to{Info:PROPERTY_NAME}` - Writes the property value to a custom property in the project information. Works only when synchronizing all elements or when synchronizing selected elements.

### _v1.6_ Line Morph Processing

| Command                                                                      | Description                                            |
| ---------------------------------------------------------------------------- | ------------------------------------------------------ |
| `Sync_from{Morph:L}`                                                         | Total length of the line morph                         |
| `Sync_from{Morph:Lx}`, `Sync_from{Morph:Ly}`, `Sync_from{Morph:Lz}`          | Length along the X/Y/Z axes                            |
| `Sync_from{Morph:Max_x}`, `Sync_from{Morph:Max_y}`, `Sync_from{Morph:Max_z}` | Maximum coordinates along the axes                     |
| `Sync_from{Morph:Min_x}`, `Sync_from{Morph:Min_y}`, `Sync_from{Morph:Min_z}` | Minimum coordinates along the axes                     |
| `Sync_from{Morph:A}`, `Sync_from{Morph:B}`, `Sync_from{Morph:ZZYZX}`         | Bounding box dimensions (similar to library elements)  |

### [Processing Construction Composition](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%92%D1%8B%D0%B2%D0%BE%D0%B4-%D1%81%D0%BE%D1%81%D1%82%D0%B0%D0%B2%D0%B0-%D0%BA%D0%BE%D0%BD%D1%81%D1%82%D1%80%D1%83%D0%BA%D1%86%D0%B8%D0%B8)

_v1.72_ To include user-defined properties of a building material, add `{@property:buildingmaterialproperties}` to the description.

| Command                                           | Description                                                                 |
| ------------------------------------------------- | ------------------------------------------------------------------------ |
| `Sync_from{Material:Layers; "TEMPLATE"}`            | Outputs the composition of construction layers                                          |
| `Sync_from{Material:Layers,PEN_NUMBER; "TEMPLATE"}` | Composition of the profile at the intersection with the pen line _<br>(v1.6)_            |
| `Sync_from{Material:Layers_inv; "TEMPLATE"}`        | Outputs layers in reverse order _<br>(v1.70)_                             |
| `Sync_from{Material:Layers_auto; "TEMPLATE"}`       | Auto-detects layer order (as in preview) _<br>(v1.70)_        |
| `Sync_from{Material:Layers; "TEMPLATE <FORMULA>"}`  | Output with calculation of expressions inside `<>` _<br>(v1.72)_                  |
| `Sync_from{Material:Layers,all; "TEMPLATE"}`        | Outputs all construction layers (auto-detects thicknesses) _<br>(v1.77)_      |
| `Sync_from{Material:Layers,unic; "TEMPLATE"}`       | Outputs unique layers (merges identical materials) _<br>(v1.77)_ |

> For user-defined material properties, add `{@property:buildingmaterialproperties}` to the description

### _v1.72_ Element Layer Modification

- `Sync_to{Attribute:Layer}` - search is performed by the full layer name (including extension)

### _v1.77+_ Getting Element Information

- `Sync_from{Element:Material overridden}` - Is material replacement enabled or disabled?
- `Sync_from{Element:Zone manual poly}` - Is the Zone created manually, with all edges static? _v1.78_
- `Sync_from{Element:zone by ref line}` - Is the Zone surrounded by the reference line of Walls, not by their side lines? _v1.78_

### _v1.77_ MEP Element Information Retrieval

Processes pipes, ducts, fittings, bends, and runs.
[Example file for AC28](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/files/MEP_28.pln)
| Command | Description |
|---------|----------|
| `Sync_from{MEP:reference set name}` | Name of the main table with diameters |
| `Sync_from{MEP:element set name}` | Name of the element table (pipe/fitting/transition) |
| `Sync_from{MEP:width}` | Cross-section width |
| `Sync_from{MEP:height}` | Cross-section height |
| `Sync_from{MEP:wall thickness}` | Wall thickness (pipes/ducts) |
| `Sync_from{MEP:bend radius}` | Bend radius |
| `Sync_from{MEP:description}` | Description from the settings table |
| `Sync_from{MEP:shape}` | Cross-section shape |
| `Sync_from{MEP:length}` | Element length (does not apply to bends) |
| `Sync_from{MEP:insulation thickness}` | Insulation thickness |
| `Sync_from{MEP:system}` | System name |
| `Sync_from{MEP:routing length}` | Length of the routing segment containing the element |
| `Sync_from{MEP:physical system name}` | _v1.78+_ System name in the MEP Systems Browser |
| `Sync_from{MEP:physical system group name}` | _v1.78+_ System group name in the MEP Systems Browser |

### _v1.72_ [Element Classification](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%90%D0%B2%D1%82%D0%BE%D0%BC%D0%B0%D1%82%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%BE%D0%B5-%D0%BA%D0%BB%D0%B0%D1%81%D1%81%D0%B8%D1%84%D0%B8%D1%86%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%BE%D0%B2,-%D0%B2%D1%8B%D0%B2%D0%BE%D0%B4-%D0%B8%D0%BD%D1%84%D0%BE%D1%80%D0%BC%D0%B0%D1%86%D0%B8%D0%B8-%D0%BE-%D0%BA%D0%BB%D0%B0%D1%81%D1%81%D0%B5)

- Enabling classification for unclassified elements - add `some_stuff_class` to the class description.

- `Sync_to{Class:CLASSIFICATION_NAME}` Text property. Search is performed by Class Id.

- `Sync_from{Class:CLASSIFICATION_NAME; FullName}` - outputs the full class name

## [Synchronization of Hierarchical Structure Properties (Curtain Wall, Elements in Zone)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%A1%D0%B8%D0%BD%D1%85%D1%80%D0%BE%D0%BD%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F-%D1%81%D0%B2%D0%BE%D0%B9%D1%81%D1%82%D0%B2-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%B0-%D1%81%D0%BE-%D1%81%D0%B2%D0%BE%D0%B9%D1%81%D1%82%D0%B2%D0%B0%D0%BC%D0%B8-%D0%B4%D1%80%D1%83%D0%B3%D0%B8%D1%85-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%BE%D0%B2)

Similar to synchronization within a single element, with the addition of the *sub suffix. For example, `Sync_from_sub{Property:GROUP_NAME/PROPERTY_NAME}` written in a Curtain Wall will read the property from a nested element (panel, frame, accessory). Similarly, `Sync_to_sub{Property:GROUP_NAME/PROPERTY_NAME}` works.

## Reset Property Value to Default

- `Sync_reset` - Resets the property everywhere - in placed elements, in favorites, on layouts - everywhere. Synchronization of the reset property is disabled while `Sync_reset` is present in the description. *Does not work in AC27 version*

## [Rounding Dimensions](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%A4%D1%83%D0%BD%D0%BA%D1%86%D0%B8%D0%B8-%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D1%8B-%D1%81-%D1%80%D0%B0%D0%B7%D0%BC%D0%B5%D1%80%D0%B0%D0%BC%D0%B8)

## [Renumbering](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%9F%D0%B5%D1%80%D0%B5%D0%BD%D1%83%D0%BC%D0%B5%D0%B5%D1%80%D0%B0%D1%86%D0%B8%D1%8F-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%BE%D0%B2)

[Example file for AC25](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/files/%D0%9D%D1%83%D0%BC%D0%B5%D1%80%D0%B0%D1%86%D0%B8%D1%8F_25.pln)

_v1.74_ If only one element is selected, all elements from visible layers for which the selected element's numbering rules are applicable will be processed.

After numbering, synchronization of the selected elements is automatically launched.

**Flag to enable numbering in one of the formats**

- `Renum_flag{property name with rule}`

- `Renum_flag{property name with rule ; NULL}`

- _v1.6_ `Renum_flag{property name with rule ; SPACE}`

- `Renum_flag{property name with rule ; ALLNULL}`

**Rule property for position in one of the formats**

- `Renum{criterion property name}`

- `Renum{criterion property name; subdivision property name}`

## Summation

[Example file for AC25](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/files/%D0%A1%D1%83%D0%BC%D0%BC%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5_%D0%90%D0%A125.pln)

Sums the property values of elements sharing the same criterion property.

Numerical values are summed; text values are concatenated, including only unique values. By default, the separator is a semicolon.
A summation flag is not required; values of all selected elements are summed. After summation, synchronization of the selected elements is automatically initiated.

_v1.74_ If only one element is selected, all elements on visible layers for which the selected element's summation rules are applicable will be processed.

- `Sum{Property:GROUP_NAME/SUMMATION_PROPERTY_NAME; Property:GROUP_NAME/CRITERION_PROPERTY_NAME}`

- `Sum{Property:GROUP_NAME/SUMMATION_PROPERTY_NAME; Property:GROUP_NAME/CRITERION_PROPERTY_NAME; separator}` to specify a separator other than ;

- _v1.72_ `Sum{Property:GROUP_NAME/SUMMATION_PROPERTY_NAME; Property:GROUP_NAME/CRITERION_PROPERTY_NAME; max}` - outputs the maximum value, for numerical properties

- _v1.72_ `Sum{Property:GROUP_NAME/SUMMATION_PROPERTY_NAME; Property:GROUP_NAME/CRITERION_PROPERTY_NAME; min}` - outputs the minimum value, for numerical properties
  ![Summation example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/sum-ru.PNG)
  ![Summation example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/sum_2-ru.PNG)