# map-pathfinding
this is a pathfinding algorithm

OSM.PBF Loader
this will convert the OSM.PBF into a binary to be used for the c program, this is only required to do once and the binary can be called back to after that.

to run this use:
    "python scripts/import_osm.py <input.osm.pbf> <output.bin>"

[Dependancies]
    - make
    - osmium