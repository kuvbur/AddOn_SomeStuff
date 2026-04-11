## Output to Construction Composition Properties

![Example Menu](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/sync_mat_master-ru.PNG)

[Video example on YouTube](https://www.youtube.com/watch?v=4k18thw7Gso)

[Example file for AC25](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/files/%D0%A1%D0%BE%D1%81%D1%82%D0%B0%D0%B2%20%D0%BA%D0%BE%D0%BD%D1%81%D1%82%D1%80%D1%83%D0%BA%D1%86%D0%B8%D0%B8%20%D1%81%20%D0%BE%D0%B1%D1%8A%D1%91%D0%BC%D0%B0%D0%BC%D0%B8_25.pln)

## General Information

To output the layer composition in properties, specify `Sync_from{Material:Layers; " TEMPLATE_STRING "}` in the property description.

The layer order is determined as follows: for multi-layer constructions - by the order of layers in the editor (**top layer first**); for complex profiles - by the position of the circle (**the circle is placed on the side of the first layer**).

To change the layer order, use `Sync_from{Material:Layers_inv; " TEMPLATE_STRING "}`.

To output layers in the same order as they appear in the Cross-Section 2D Preview, use `Sync_from{Material:Layers_auto; " TEMPLATE_STRING "}`.

In the template string, specify the information to be output for a construction LAYER. You can use both property values and static text. Property names must be enclosed in percent signs %. _**For example:**_
_**`"%Description% - %Thickness.2mm%mm. "`**_

To output thickness, specify `%Thickness%` or `%layer_thickness%` in the template.

To output the layer number, use `%n%`. When changing the order via Layers_inv or Layers_auto, the numbering changes accordingly. \_v1.77+* To skip layers, you can use `%ns%`, which numbers only non-empty strings.

Any property can be used in the template string - both system and user-defined (including properties from project information).

_v1.72_ By default, the property is assumed to be for the element. If you need to output a user-defined property of the material, add `{@property:buildingmaterialproperties}` to the description of that property.

## Composition of Walls/Columns/Beams Made with Complex Profiles

`Sync_from{Material:Layers,LAYER_NUMBER; " TEMPLATE_STRING "}`
![Profile composition output](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/material_layers-ru.PNG)

## [Rounding of Thicknesses and Other Numerical Properties](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%9E%D0%BA%D1%80%D1%83%D0%B3%D0%BB%D0%B5%D0%BD%D0%B8%D0%B5-%D1%82%D0%BE%D0%BB%D1%89%D0%B8%D0%BD-%D0%B8-%D0%BE%D1%81%D1%82%D0%B0%D0%BB%D1%8C%D0%BD%D1%8B%D1%85-%D1%87%D0%B8%D1%81%D0%BB%D0%B5%D0%BD%D0%BD%D1%8B%D1%85-%D1%81%D0%B2%D0%BE%D0%B9%D1%81%D1%82%D0%B2)

## Line Wrapping (Setting Fixed Text Length for Each Layer)

Since line breaks do not work in interactive catalogs, you can use a maximum character count setting. To do this, add `~CHAR_COUNT` to pad with spaces or `@CHAR_COUNT` to pad the line with a tab character to the rule. A space must follow the character count. For example: `Sync_from{Material:Layers_auto, 20; "%Description% - %Thickness.2mm%mm. ~200 "}`

If the property output is intended only for a label, you can use the following special characters for formatting:

- `\TAB` - Character Tabulation, U+0009
- `\CRLF` - Carriage Return + Line Feed, U+000D U+000A
- `\CR` - Carriage Return, U+000D
- `\LF` - Line Feed, U+000A
- `\PS` - Paragraph Separator,
- `\LS` - Line Separator, U+2028
- `\NEL`, Next Line, U+0085
- `\NL` - Symbol For Newline, U+2424

For example: `Sync_from{Material:Layers_auto, 20; "%Description% - %Thickness.2mm%mm. \CRLF"}`

## Assigning Special Text for Individual Materials

Create a property available in the material classification. In the property description, specify `Sync_name`. Ensure that this property is visible for the material. To selectively disable the special text, add the text `%nosyncname%` to the relevant rule.

One way to remove a material from the composition is to put a space in the special text property.

![Example Menu](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/sync_mat_mater-ru.PNG)

![Example Menu](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/sync_mat_mater1-ru.PNG)

## _v1.72_ Formula Assignment

If a template string contains the characters `<` and `>`, the entire template string is interpreted by the add-on as a formula for calculation. In this case, the part of the expression enclosed between these characters will be duplicated for each layer. For example, for a wall with layers 20 (plaster) + 380 (brick) + 20 (plaster), the expression `<+2*%thickness.1mm%>` will be interpreted as `+2*20+2*380+2*20`, which results in 840. The expression `"1+<+2*%thickness.1mm%>"` will yield the result `1+(+2*20+2*380+2*20)`. First, the add-on substitutes numerical values into the template string and then evaluates the resulting expression. Therefore, a notation like `<2*%thickness.1mm%>`, without an arithmetic operator after the opening bracket `<`, is meaningless and will not be evaluated.

## _v1.76_ Format for Outputting Composition with a Callout [BeArt Label mod](https://github.com/kuvbur/gdl_bibl/tree/master/kuvbur_%D0%92%D1%8B%D0%BD%D0%BE%D1%81%D0%BA%D0%B8%20%D0%B8%20%D1%88%D1%82%D0%B0%D0%BC%D0%BF)

Format m[ ]@t[ ], for example `Sync_from{Material:Layers,20; "m[%штриховка%]t[%толщина.2mm%]@"}`

Format b[ ]@t[ ], for example `Sync_from{Material:Layers,20; "b[%bmat_inx%]t[%толщина.2mm%]@"}`

## _v1.77+_ Output Component Quantity to Property

!! ATTENTION !! The component volume depends on the layer intersection group numbers.
To use all pens, replace the pen number with `all`, e.g., Sync_from{Material:Layers,**all**;
To merge layers by thickness and material index, replace the pen number with `unic`, e.g., Sync_from{Material:Layers,**unic**;

- `%area%` - area, in sq.m.
- `%volume%` - volume, in cu.m.
- `%length%` - length, in m.
- `%width%` - width, in m.
- `%qty%` - material quantity according to the unit of measurement (area for sq.m., volume for cu.m.). Will be multiplied by the waste factor if it is specified.
- `%unit%` - unit of measurement, specified in the material property
- `%unit_prefix%` - outputs S, V, or L depending on the selected unit of measurement

### Building Material Properties for Quantity Output

Properties must be visible in the material classification and materials must be classified. Add the following descriptions to the properties:

- `some_stuff_units` - Unit of measurement. Possible options: `sq.m.`, `m²`, `cubic m.`, `m³`, `linear m.`, `not specified`. Depending on the value of this property, the `%qty%` parameter will output either area or volume. Using `%area%`, `%volume%` is not required.
- `some_stuff_th` - Optional property. Thickness for materials with constant thickness. For example, waterproofing layers. Type - length. If thickness is set via properties, the system thickness from the composite structure/profile will be ignored. Does not affect the displayed thicknesses within the structure composition.
- `some_stuff_kzap` - Optional property. Safety factor. Affects only the output to the `%qty%` variable.

## Output of Material Properties

In addition to user-defined properties, construction data can include information about building materials.

_**For example, the string `Sync_from{Material:Layers; "%Описание% - %Толщина.2mm%мм. "}` is equivalent to `Sync_from{Material:Layers; "%BuildingMaterialProperties/Building Material Description% - %layer_thickness.2mm%mm. "}`**_

Property names must be enclosed in percent signs %, e.g., %BuildingMaterialProperties/Building Material Description%.

- `%material class name%` or `%material class name; CLASSIFICATION_NAME%` - building material class name _v1.78+_

- `%material class id%` or `%material class id%; CLASSIFICATION_NAME%` - building material class ID _v1.78+_

- `%material class description%` or `%material class description%; CLASSIFICATION_NAME%` - building material class description _v1.78+_

- `%n%` - layer number (all layers are numbered)

- `%ns%` - layer number (only layers with a non-empty template string are numbered) _v1.77+_

- `%layer_thickness%` - equivalent to `%Толщина%"`

- `%th%` - equivalent to `%Толщина%"` _v1.77+_

- `%unit%` - outputs the unit of measurement, from the property with description `some_stuff_units` _v1.77+_

- `%kzap%` - safety factor, from the property with description `some_stuff_kzap` _v1.77+_

- `%area%` - component area _v1.77+_

- `%volume%` - component volume _v1.77+_

- `%qty%` - component quantity according to the selected unit of measurement (area or volume) _v1.77+_

- `%unit_prefix%` - outputs S if the unit is sq.m., or V for cu.m. _v1.77+_

- `%bmat_inx%` - building material index

- `%cutfill_inx%` - fill pattern index

- `%BuildingMaterialProperties/Building Material ID%` - equivalent to `%id%`

- `%BuildingMaterialProperties/Building Material Name%` - `%наименование%`

- `%BuildingMaterialProperties/Building Material Description%` - `%описание%`

- `%BuildingMaterialProperties/Building Material Manufacturer%` - `%производитель%`

- `%BuildingMaterialProperties/Building Material Density%` - `%плотность%`

- `%BuildingMaterialProperties/Building Material CutFill%` - fill pattern name, equivalent to `%штриховка%`

- `%BuildingMaterialProperties/Attribute Name%`

- `%BuildingMaterialProperties/Building Material Embodied Carbon%`

- `%BuildingMaterialProperties/Building Material Embodied Energy%`

- `%BuildingMaterialProperties/Building Material Heat Capacity%`

- `%BuildingMaterialProperties/Building Material Thermal Conductivity%`

- `%BuildingMaterialProperties/Building Material CutFill Foreground Pen%`

- `%BuildingMaterialProperties/Building Material CutFill Background Pen%`

- `%BuildingMaterialProperties/Building Material CutMaterial%`

- `%BuildingMaterialProperties/Building Material Participates in Collision Detection%`

- `%MaterialProperties/Material Vectorial Hatching Fill Index%`

- `%MaterialProperties/Material Vectorial Hatching Pen Index%`

---

List of built-in property names for AC22-AC27

- `%generalelemproperties/elementid%`

- `%generalelemproperties/hotlinkandelementid%`

- `%generalelemproperties/hotlinkmasterid%`

- `%generalelemproperties/hotlink source id%`

- `%generalelemproperties/home story%`

- `%generalelemproperties/layerindex%`

- `%generalelemproperties/layercombinationindex%`

- `%generalelemproperties/missing attributes%`

- `%generalelemproperties/property object name%`

- `%generalelemproperties/uniqueid%`

- `%generalelemproperties/archicadifcid%`

- `%generalelemproperties/external ifc id%`

- `%generalelemproperties/last issue date%`

- `%generalelemproperties/last issue id%`

- `%generalelemproperties/last issue name%`

- `%generalelemproperties/listing label text%`

- `%generalelemproperties/locked%`

- `%generalelemproperties/linked changes%`

- `%generalelemproperties/type%`

- `%generalelemproperties/related zone category%`

- `%generalelemproperties/related zone name%`

- `%generalelemproperties/colliding zones names%`

- `%generalelemproperties/related zone number%`

- `%generalelemproperties/elevation to story%`

- `%generalelemproperties/is old stair%`

- `%generalelemproperties/surface list from elem%`

- `%generalelemproperties/parent id%`

- `%generalelemproperties/general_line_type%`

- `%generalelemproperties/general_opening_ids%`

- `%generalelemproperties/top elevation to project zero%`

- `%generalelemproperties/top elevation to first reference level%`

- `%generalelemproperties/top elevation to home story%`

- `%generalelemproperties/top elevation to second reference level%`

- `%generalelemproperties/top elevation to sea level%`

- `%generalelemproperties/bottom elevation to project zero%`

- `%generalelemproperties/bottom elevation to first reference level%`

- `%generalelemproperties/bottom elevation to home story%`

- `%generalelemproperties/bottom elevation to second reference level%`

- `%generalelemproperties/bottom elevation to sea level%`

- `%generalelemproperties/opening number%`

- `%generalelemproperties/layer name%`

- `%generalelemproperties/surface name list%`

- `%generalelemproperties/surface name list from elem%`

- `%generalelemproperties/general_owner_id%`

- `%generalelemproperties/general_axis_line_pen%`

- `%generalelemproperties/general_axis_line_type%`

- `%generalelemproperties/general_cover_fill_type%`

- `%generalelemproperties/general_overhead_line_type%`

- `%generalelemproperties/general_uncut_line_type%`

- `%generalelemproperties/general_cut_line_type%`

- `%generalelemproperties/general_cover_fill_background_pen%`

- `%generalelemproperties/general_overhead_line_pen%`

- `%generalelemproperties/general_uncut_line_pen%`

- `%generalelemproperties/general_thickness%`

- `%generalelemproperties/general_font_size_list%`

- `%generalelemproperties/general_font_style_list%`

- `%generalelemproperties/general_alignment_list%`

- `%generalelemproperties/general_profile_index%`

- `%generalelemproperties/general_leading_list%`

- `%generalelemproperties/general_font_id_list%`

- `%generalelemproperties/general_width%`

- `%generalelemproperties/general_text_pen_index_list%`

- `%generalelemproperties/general_cover_fill_pattern_pen%`

- `%generalelemproperties/cut_fill_outline_pen%`

- `%generalelemproperties/fill_outline_pen%`

- `%generalelemproperties/fill_foreground_pen%`

- `%generalelemproperties/complex profile name%`

- `%generalelemproperties/cut_fill_background_pen%`

- `%generalelemproperties/fill_background_pen%`

- `%generalelemproperties/top offset%`

- `%generalelemproperties/relative top link story%`

- `%generalelemproperties/absolute top link story%`

- `%generalelemproperties/top link story%`

- `%generalelemproperties/general composite%`

- `%generalelemproperties/composite name%`

- `%generalelemproperties/general building material from elem%`

- `%generalelemproperties/building material id from elem%`

- `%generalelemproperties/general_2d_outline_pen%`

- `%generalelemproperties/fill index list%`

- `%generalelemproperties/general elevation to project zero%`

- `%generalelemproperties/general building material%`

- `%generalelemproperties/building material id%`

- `%generalelemproperties/elevation to second reference level%`

- `%generalelemproperties/elevation to sea level%`

- `%generalelemproperties/elevation to first reference level%`

- `%generalelemproperties/elevation to current story%`

- `%generalelemproperties/general_pen_index_list%`

- `%generalelemproperties/general_3d_length%`

- `%generalelemproperties/building material name from elem%`

- `%generalelemproperties/building material name%`

- `%generalelemproperties/home offset%`