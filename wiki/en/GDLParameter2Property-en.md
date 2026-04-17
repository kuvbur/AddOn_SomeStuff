## Synchronization of GDL Parameters and Properties

The add-on copies the value of a parameter or property into a property whose **description** contains `Sync_from{PARAMETER_NAME}` or `Sync_to{PARAMETER_NAME}`.

- `Sync_from{PARAMETER_NAME}` - copies values FROM the parameter INTO the property with this description
- `Sync_to{PARAMETER_NAME}` - copies values FROM the property with this description INTO the parameter

### Control Commands

- **Track** - starts monitoring element changes. Copying occurs when an element is modified.
- **Synchronize All** - copies values for all elements of the selected type.
- **Synchronize Selected** - copies values only for selected elements (without activating monitoring).

### Element Filtering

- Use menu items:  
  **Process Walls/Slabs**  
  **Process Windows/Doors**  
  **Process Objects**
- To exclude an element from processing, create a property with the description `Sync_flag` (type: match criterion).

---

### Copying Parameters to Properties

_Applicable for library elements, windows, doors, zones:_

1. Select the library element
2. Open the object editor:
   - `Ctrl+Shift+O` or  
     [File > Libraries and Objects > Open Object](https://helpcenter.graphisoft.com/ru/user-guide/67251/)
   - For password-protected objects, use Dump Selected Library Parts. It will output a report listing all parameters.
     ![Dump](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/dump-ru.PNG)
3. Click the [Parameters](https://helpcenterint-wpengine.netdna-ssl.com/ru/wp-content/uploads/sites/6/archicad-22/130_userinterfacedialogboxes/SearchObjectParameters-ru.PNG) button
4. Find the parameter in the **Name** column, copy its identifier from the **Variable** column
5. In the Property Manager:
   - In the **description** of the target property, enter `Sync_from{copied_identifier}`  
     ![Example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/add3-ru.PNG)
   - Create a flag property with the description `Sync_flag` (type: criteria). Pay attention to
     ![Example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/add2-ru.PNG)
6. Make the properties available in the classes you need
   ![Example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/add5-ru.PNG)
7. Run the required command

> Data type is automatically converted (number ↔ string)

---

### Copying Properties to Properties

_Applicable to all element types:_

1. In the Property Manager:
   - In the **description** of the target property, specify `Sync_from{Property:GROUP_NAME/PROPERTY_NAME}`
     ![Example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/add4-ru.PNG)
   - Create a flag property with the description `Sync_flag`
     ![Example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/add2-ru.PNG)
2. Run the required command

> To search for properties, use the format `Property:Group/Property`.

## [Nothing Works. What Should I Do?](https://github.com/kuvbur/AddOn_SomeStuff/wiki/FAQ#%D0%B0%D0%B4%D0%B4%D0%BE%D0%BD-%D1%83%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BB%D0%B5%D0%BD-%D0%BD%D0%BE-%D0%BD%D0%B8%D1%87%D0%B5%D0%B3%D0%BE-%D0%BD%D0%B5-%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D0%B0%D0%B5%D1%82-%D1%87%D1%82%D0%BE-%D0%B4%D0%B5%D0%BB%D0%B0%D1%82%D1%8C)