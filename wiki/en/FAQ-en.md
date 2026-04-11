### Why This Add-on?

The add-on solves several highly specialized tasks.

- **Property and GDL Connection**: Two-way exchange between properties and GDL parameters
- **Flexible Numbering**: Analog of the ID Manager with recording positions in properties
- **Structure Analysis**: Output of complex profile composition in any section
- **Summation in Properties**: Analog of the IK function with result preservation
- **Geometry Control**: Checking coordinates/angles + output to properties
- **Morph Line Length**: Determining the length of morph lines
- **Element Orientation**: Determining cardinal directions for walls/openings
- **Mathematical Operations**: Rounding dimensions, formulas (6x100=600)
- **IFC Integration**: Copying IFC properties to standard properties
- **Project Data**: Reading/writing project information to properties
- **Class Automation**: Assigning classification based on property values
- **Layer Management**: Assigning layers based on property values

### The Add-On is Installed, but Nothing Works. What to Do?

Go through the checklist and verify that everything is in order.

1. The element is classified. Read about classification and its creation [here](https://help.graphisoft.com/AC/25/RUS/index.htm#t=_AC25_Help%2F045_PropertiesClassifications%2F045_PropertiesClassifications-1.htm%23XREF_15029_Element_Properties&rhsearch=%D0%BA%D0%BB%D0%B0%D1%81%D1%81%D0%B8%D1%84%D0%B8%D0%BA%D0%B0%D1%86%D0%B8%D1%8F&rhsyns=%20). You can use [auto-classification](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%90%D0%B2%D1%82%D0%BE%D0%BC%D0%B0%D1%82%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%BE%D0%B5-%D0%BA%D0%BB%D0%B0%D1%81%D1%81%D0%B8%D1%84%D0%B8%D1%86%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%BE%D0%B2,-%D0%B2%D1%8B%D0%B2%D0%BE%D0%B4-%D0%B8%D0%BD%D1%84%D0%BE%D1%80%D0%BC%D0%B0%D1%86%D0%B8%D0%B8-%D0%BE-%D0%BA%D0%BB%D0%B0%D1%81%D1%81%D0%B5).
2. The flag property is available for the element. A flag property is a property whose description contains `Sync_flag`. Available means this property is visible in the element's parameters. More about configuration is [written here](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%A1%D0%B8%D0%BD%D1%85%D1%80%D0%BE%D0%BD%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F-%D0%BF%D0%B0%D1%80%D0%B0%D0%BC%D0%B5%D1%82%D1%80%D0%BE%D0%B2-%D0%B8-%D1%81%D0%B2%D0%BE%D0%B9%D1%81%D1%82%D0%B2#%D0%BA%D0%BE%D0%BF%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5-%D0%B7%D0%BD%D0%B0%D1%87%D0%B5%D0%BD%D0%B8%D0%B9-%D0%BF%D0%B0%D1%80%D0%B0%D0%BC%D0%B5%D1%82%D1%80%D0%B0-%D0%B2-%D1%81%D0%B2%D0%BE%D0%B9%D1%81%D1%82%D0%B2%D0%B0-%D0%BF%D1%80%D0%B8%D0%BC%D0%B5%D0%BD%D0%B8%D0%BC%D0%BE-%D1%82%D0%BE%D0%BB%D1%8C%D0%BA%D0%BE-%D0%B4%D0%BB%D1%8F-%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D1%87%D0%BD%D1%8B%D1%85-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%BE%D0%B2-%D0%BE%D0%BA%D0%BE%D0%BD-%D0%B4%D0%B2%D0%B5%D1%80%D0%B5%D0%B9-%D0%B7%D0%BE%D0%BD).
3. The element has only one flag property.
4. The flag property has a value of TRUE.
5. The properties the add-on should work with are visible in the element's parameters.
6. Ensure processing for the corresponding element type is enabled.
   ![Enabling Types](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/sync_type-en.png)
7. Check the syntax of the property description into which the parameter value is copied (the property whose description contains `Sync_from`). The brackets must be curly. There should be no spaces before the brackets (`Sync_from{` not `Sync_from {`). You can copy the description [from here](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%A1%D0%BF%D0%B8%D1%81%D0%BE%D0%BA-%D0%B2%D0%BE%D0%B7%D0%BC%D0%BE%D0%B6%D0%BD%D1%8B%D1%85-%D0%BA%D0%BE%D0%BC%D0%B0%D0%BD%D0%B4-%D0%B2-%D0%BE%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D1%8F%D1%85-%D1%81%D0%B2%D0%BE%D0%B9%D1%81%D1%82%D0%B2), substituting the required parameter.
8. If tracking is disabled (the Track button is not active) - the add-on runs manually. You need to select the elements and click Synchronize Selected.
9. Look at the Report in the Navigator panel - the add-on outputs the main information about performed operations there.
   ![Report](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/report-en.png)

### Can an Add-on Slow Down ArchiCAD's Performance?

Briefly - yes, it can, if tracking is enabled and a large number of elements are modified simultaneously. The add-on triggers only when an element is changed (if tracking is enabled) and during manual execution. It does not affect rendering speed or performance in the 3D window.
To improve performance, it is recommended:

- Limit the visibility of synchronized properties. For example, it is not advisable to make a property with the number of sockets visible for Walls and partitions.
- During major operations (moving a large number of elements, inserting from a new file), it makes sense to disable tracking and perform synchronization of all elements later, at a convenient time.
- Disable tracking for unused types (e.g., curtain wall elements)

The number of synchronized properties plays a minor role in this - synchronizing 1000 elements with one property will take approximately the same time as synchronizing the same number of elements with 10 properties.