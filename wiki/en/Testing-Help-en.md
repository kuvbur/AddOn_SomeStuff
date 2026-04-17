# Working with Add-On Test Versions

If you wish and are able to participate in testing new versions of the add-on, you must do the following:

- Register on GITHUB. Registration is free; without registration, you will not be able to download intermediate builds.
- Go to the `Actions` section.
- Find the topmost line with the label `Add-On Build` and click on it.
- Download the required package and install it in the usual way.

![Example 1](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/test1-ru.PNG)
![Example 2](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/test2-ru.PNG)
![Example 3](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/test3-ru.PNG)

# Modified Functions for Testing in Version 1.77 (as of 07/10/2025)

**New Functions**

- [Finishing Schedule Calculation and Finishing Element Creation](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Finishing-en)
- [Output of Component Volumes into a Label](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Construction-Composition-en)
- [Output of Element PROPERTY Information](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Property-Commands-List-en)
- [Creation of 3D Documents Along a Morph-Line or Fence](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Build-Profile-Line-en)
- Alignment of Drawings on Layouts to Hotspots
- Added classic rounding to dimensions (add `ClassicRound` to the rounding rule) #133
- Added attribute name search when writing to a GDL Parameter #134
- Added re-synchronization when using properties processed by the add-on in formulas #135
- Added selection of connected child elements #138
- Added message display when calling Show Related Elements (if some elements are on another story or do not exist) #139
- Added add-on version output to the report, added error message output during element creation.
- Added processing to replace double spaces and trim spaces in unique parameters when creating a schedule (e.g., elements with only a space in a parameter will be considered as having no space. Double spaces in calculations will be replaced with single spaces).
- Added output of the replacement coverage value for objects (including windows and doors) #137
- Added a second version of the Create Elements function (`Spec_rule_v2`)
- Summation of text values when outputting Create Elements functions now outputs the full list of values, not just unique ones.

**Bug Fixes**

- Element linking may fail if the project has several properties with the description `Sync_GUID` that are not visible for the element being linked #132
- Excluded processing of openings during dimension rounding due to a bug in ArchiCAD. Dimensions where at least one point is attached to an opening (Opening) will not be processed.