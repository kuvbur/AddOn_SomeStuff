## Build Profile Along Line

Creates 3D section documents along morph lines or fences. Used for constructing network profiles according to GOST.
1. For each profile, create a worksheet with a point (hotspot) using pen number 163. This worksheet will be used for general layout.
2. Draw a morph or fence along the network route. In the morph's ID, specify `SECTION_NAME@SCALE`, for example `B1.1@500`.
3. Run the function `Build Profile Along Line`. Select the morph line/fence. 3D documents will be created for each section. In the 3D documents, points (hotspots) will be placed at the edges of the section. Do NOT delete them.
4. Save the 3D document views and place them on the layout (alignment is not necessary at this stage, they can be placed in a group).
5. On the layout, select the group of profile drawings and run the function `Align Selected Drawings`.

If the route position on the plan changes, move the morph/fence and repeat the steps above. The existing 3D documents will be updated, and new ones will be added.