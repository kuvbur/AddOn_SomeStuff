Welcome to the AddOn_SomeStuff wiki!
[FAQ](https://github.com/kuvbur/AddOn_SomeStuff/wiki/FAQ)

![Пример 1](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/image/all.PNG)
![Пример 2](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/image/all_ENG.PNG)

### Why this addon?

The addon solves several highly specialized tasks.

1. Combining the world of properties and GDL parameters. By standard means, it is impossible to output the value of the GDL object parameters to properties, as well as use the property value in the GDL code.
2. Flexible numbering of elements according to criteria. An analogue of the ID Manager, but with flexible criteria settings and entry of the position in the properties.
3. The derivation of the composition of structures into properties, including for complex profiles in any given section. This is not possible by regular means.
4. Summation of values. Similar to the standard summation function in Interactive Catalogs, but stores the sum in properties, can be used for remote labeling or calculations.
5. Checking the correctness (presence of a fractional part) of the coordinates and angles of the elements, as well as displaying coordinates in properties.
6. Output the length of the morph line.
7. Determining the cardinal directions for walls and openings.
8. Rounding dimensions, writing formulas to dimensions (for example, 6x100=600).
9. Copying IFC properties to regular properties.
10. Writing and reading project data to the element properties.

[Install](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Install)
[Commands](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Addon-command)

### The addon is installed, but nothing works. What to do?

It is necessary to go through the points and check if everything is being done.

1. The item is classified.
2. The flag property is available for the element. The flag property is a property whose description specifies Sync_flag. Available - means that this property is visible in the element parameters.
3. The flag property has the value TRUE
4. Check the syntax of the description of the property to which the parameter value is copied (the property in the description of which Sync_from is specified). The brackets must be curly. You can copy the description by substituting the desired parameter.
5. If tracking is disabled (the Track button is not active), the addon is launched manually. You need to select the items and click Synchronize selected.
