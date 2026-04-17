## Working with Dimensions (Rounding, etc.)

**Due to a bug in ArchiCAD, dimensions attached to profile columns or openings are NOT processed!**

All linear reserved visible and unlocked dimensions on the current view that fall under the rules specified in the `Addon_Dimensions` field of the project information are processed. Write rules with the separator ;.

![dim_rule.PNG](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/dim_rule-ru.PNG)

Checking and processing all dimensions on the current view occurs:

- When opening any view (switching the active window)
- Changing the floor
- Any action with a tracked element (when the Track item in the add-on menu is enabled)
- Running the command `Synchronize All`

Possible rule values, separated by semicolon ";" :

- `"LAYER" - MULTIPLE_MM, TEXT_PEN_MODIFIED, FLAG_CHANGE_CONTENT`

- `"LAYER" - MULTIPLE_MM, TEXT_PEN_MODIFIED, <FORMULA>`

- `"LAYER" - MULTIPLE_MM, TEXT_PEN_MODIFIED, FLAG_CHANGE_CONTENT, <FORMULA>`

- `DIMENSION_PEN - MULTIPLE_MM, TEXT_PEN_MODIFIED, FLAG_CHANGE_CONTENT`

- `DIMENSION_PEN - MULTIPLE_MM, TEXT_PEN_MODIFIED, <FORMULA>`

- `DIMENSION_PEN - MULTIPLE_MM, TEXT_PEN_MODIFIED, FLAG_CHANGE_CONTENT, <FORMULA>`

`DIMENSION_PEN` is the dimension line pen.

![dim_pen.PNG](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/dim_pen-ru.PNG)

Rounding is performed upward. _v1.77_ For classic rounding, add `СlassicRound` to the end of the rule, for example `".КЖ" - 5, 20, 0, СlassicRound`

When a layer name is specified, it checks whether part of the text from the rule is contained in the layer name. That is, only a fragment of the name can be written in the rule. All dimensions that are not multiples of the specified value change their color to "TEXT_PEN_MODIFIED". If the content change flag is zero, text replacement is not performed — only the pen changes.

For example, ".КЖ" - 5, 20, 0 will highlight (but not change) all dimensions not multiples of 5mm, lying on a layer containing ".КЖ", and highlight them red (pen 20)

In the formula, it is possible to use values of properties associated with the dimension elements.

_v1.72_ To hide wall thicknesses, add `DeleteWall` to the rule.

_v1.72_ To reset the value of a dimension chain to measured values, add `ResetText` to the rule. Only the values will be reset; the pen does not change.

_v1.76_ To highlight overridden dimensions, add `CheckCustom` to the rule.