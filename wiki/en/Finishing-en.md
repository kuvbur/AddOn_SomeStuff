## Finishing Schedule

[Test file](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/files/%D0%9E%D1%82%D0%B4%D0%B5%D0%BB%D0%BA%D0%B0_25.pla)

Performs a breakdown by the composition of adjacent structures, recording information about the layers of adjacent structures.

Can be used both for constructing additional finishing elements on top of existing structures (similar to the Interior Master) and for recording finishing information in the zone properties. These two mechanisms work independently - they can be used either individually or together.

### Limitations

- Does not account for zone trimming by roofs
- Does not work with solid element operations on zones/bounding structures
- For reveals, uses only data from window parameters (not from zone calculation settings)
- Does not process stairs

### Building Material Properties

The add-on analyzes wall layers with the "Finish" type, collecting data from the zone outward to the first "Core" layer (inclusive). **Note:** in constructions without a "Core" layer, ALL layers are output.

| Property                         | Type                  | Description                                                                                                                                                                |
| -------------------------------- | --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `some_stuff_layer_onoff`         | Matching Criterion    | Output layer to the finishing schedule                                                                                                                                          |
| `some_stuff_layer_has_finish`    | Matching Criterion    | Adding a finish layer from zone parameters (optional). When disabled, the finish layer from the zone is replaced by the material specified in the building material settings. |
| `some_stuff_layer_description`   | Text                  | Material name. Supports templates with `%%`                                                                                                                         |
| `some_stuff_layer_favorite_name` | Text                  | Favorite name (optional)                                                                                                                                            |

If properties are absent, the names of all finishing layers are output.

### Zone Properties

The created finishing walls will be divided in height according to the settings of the zone properties. There are three possible sections in total - panels (made of walls), the main part, and the top strip (which can be used for the space from the suspended ceiling to the slab).

#### Zone Properties for Reading

#### Element Creation Control

| Property Description             | Purpose                                  |
| -------------------------------- | ---------------------------------------- |
| `some_stuff_fin_create_elements` | main toggle for element creation         |
| `some_stuff_fin_create_ceil`     | create ceilings                          |
| `some_stuff_fin_create_floor`    | create floors                            |
| `some_stuff_fin_create_wall`     | create walls                             |
| `some_stuff_fin_create_column`   | create columns                           |
| `some_stuff_fin_create_reveal`   | create reveals                           |

Property type - matching criterion. If specialized properties are absent - the value of `some_stuff_fin_create_elements` is used. If properties are not set - elements will be created.

#### Finishing Configuration

When using the add-on together with the [library](https://github.com/kuvbur/gdl_bibl), finishing properties are read directly from the Zone parameters.
When using other zones, it is necessary to create properties with the following descriptions (i.e., create a property and add the required text to its description):

| Property Description            | Purpose                                                                                    | Data Type            |
| -------------------------------- | --------------------------------------------------------------------------------------------- | --------------------- |
| `some_stuff_fin_ceil_material`   | Ceiling finish material name (final finish)                                                       | String                |
| `some_stuff_fin_up_material`     | Upper part finish material name (difference between zone height and main finish height, final finish) | String                |
| `some_stuff_fin_main_material`   | Main wall part finish material name (final finish)                                          | String                |
| `some_stuff_fin_down_material`   | Panel finish material name (bottom of walls and columns, final finish)                                   | String                |
| `some_stuff_fin_column_material` | Column finish material name (final finish)                                                        | String                |
| `some_stuff_fin_main_height`     | Zone height up to the ceiling (if not set or equals zero - the zone height is used)             | Length                 |
| `some_stuff_fin_down_height`     | Panel height                                                                                | Length                 |
| `some_stuff_fin_has_ceil`        | Presence of a ceiling                                                                               | Criteria |
| `some_stuff_fin_floor_by_slab`   | Calculate and build ceiling only based on slabs                                               | Criteria |
| `some_stuff_fin_has_floor`       | Presence of a floor                                                                                  | Criteria |
| `some_stuff_fin_ceil_by_slab`    | Calculate and build floor only based on slabs                                                   | Criteria |
| `some_stuff_fin_type`            | Finishing type for merging zones with identical finishes                                         | String                |

#### Zone Properties for Result Recording

Used for outputting results without constructing finishing elements. All finishing layers are recorded into a single cell.
The property type for result recording is string. If one of the mandatory properties is not found, recording to the others is not performed. Area calculation is performed without considering intersections/subtractions of finishing walls.

| Property Description                              | Purpose                                              | Note                              |
| -------------------------------------------------- | ------------------------------------------------------- | --------------------------------------- |
| `some_stuff_fin_ceil_result`                       | Ceiling finish                                         | Mandatory property                   |
| `some_stuff_fin_up_result`                         | Upper wall finish, from suspended ceiling to top of wall | If not specified - will be added to the wall column |
| `some_stuff_fin_main_result`                       | Wall finish                                            | Mandatory property                   |
| `some_stuff_fin_down_result`                       | Lower wall/column finish                                | Mandatory property                   |
| `some_stuff_fin_column_result`                     | Column finish                                          | Mandatory property                   |
| `some_stuff_fin_reveal_result`                     | Reveal finish                                         | If not specified - will be added to the wall column |
| Result recording with grouping by finish type |
| `some_stuff_fin_up_result_bytype`                  | Upper wall finish, from suspended ceiling to top of wall | If not specified - will be added to the wall column |
| `some_stuff_fin_main_result_bytype`                | Wall finish                                            | Mandatory property                   |
| `some_stuff_fin_down_result_bytype`                | Lower wall/column finish                                | Mandatory property                   |
| `some_stuff_fin_column_result_bytype`              | Column finish                                          | Mandatory property                   |
| `some_stuff_fin_reveal_result_bytype`              | Reveal finish                                         | If not specified - will be added to the wall column |

#### Output Formatting (Font, Font Size)

Since all materials along with the area are written into one cell, the add-on adjusts line breaks based on data about the font and the expected column widths. By default, the add-on is configured to work with GOST 2.304 type A font, font size 3, width for material 40mm, for area 20mm (total column width in the IK - 60mm). To specify other values, add them to the description of the desired output column in the format `{MATERIAL_WIDTH ; AREA_WIDTH ; SIZE ; FONT}`, for example `{ 50 ; 30 ; 3 ; ISOCPEUR }`

## Settings for Created Finishing Elements

These settings are required only for creating finishing elements and do not affect writing the finishing schedule to the zone.

### Classification of Finishing Elements

For the add-on to function, it is necessary to configure the classes for finishing elements. The configuration involves adding the following text to the Class Description:

- `some_stuff_fin_class` - General class for all elements. Assigned if other classes are not found.
- `some_stuff_fin_walls` - For walls (if not specified, it will take the `some_stuff_fin_class`)
- `some_stuff_fin_down_walls` - For wall bases (if not specified, it will take the `some_stuff_fin_class`)
- `some_stuff_fin_reveals` - For reveals (if not specified, it will take the `some_stuff_fin_wall` or `some_stuff_fin_class`)
- `some_stuff_fin_columns` - For columns (if not specified, it will take the `some_stuff_fin_wall` or `some_stuff_fin_class`)
- `some_stuff_fin_floors` - For floors (if not specified, it will take the `some_stuff_fin_class`)
- `some_stuff_fin_ceils` - For ceilings (if not specified, it will take the `some_stuff_fin_class`)

For the add-on to work correctly, the existence of the class with the description `some_stuff_fin_class` is sufficient; other classes can be created as needed. The add-on itself assigns the classification to elements during the creation of finishing elements; manual assignment is not required. Elements classified as finishing are not considered in calculations.

### Properties of Finishing Elements

Properties must be visible in the classification of finishing elements

- `Sync_GUID base element` - for the add-on to record the GUID of the base element to which the finishing element will be linked
- `Sync_GUID zone` - for the add-on to record the GUID of the zone

### Properties of Base Elements

- `some_stuff_element_has_finish` - instead of applying the zone's finish, the material (coating) of the element's last layer will be applied.
- `some_stuff_element_onoff` - completely disables the element's processing by the finish schedule.

### Creating Finishing Elements from Favorites

The add-on checks whether the name of a finishing material in a zone contains a reference to a favorite name. The material name is formed in the format `MATERIAL_NAME@FAVORITE_NAME`, for example `Paint Coating@Wall` - where `Paint Coating` is the material name in the finishing schedule, and `Wall` is the name of the element in the favorites.

If no favorite is specified for the material (i.e., the material name does not contain the `@` symbol or a favorite with that name is not found), the add-on will search for favorite elements based on the type of surface being finished:

- `smstf window` - opening for insertion into finishing walls at window and door locations. Tool type - window
- `smstf wall` - finishing of vertical surfaces. Tool type - wall or object (accessory)
- `smstf ceil` - for modeling ceilings. Tool type - slab or object (accessory)
- `smstf floor` - for modeling floors. Tool type - slab or object (accessory)
- `smstf reveal side` - for modeling the vertical part of reveals. Tool type - wall or object (accessory)
- `smstf reveal up` - for modeling the horizontal part of reveals. Tool type - beam

If no suitable elements are found in the favorites at all, the tool's default settings will be applied (for walls and slabs).