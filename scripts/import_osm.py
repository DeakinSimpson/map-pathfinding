import osmium
import struct
import sys
import math
import re

ROAD_TYPES = {
    "motorway",
    "motorway_link",
    "trunk",
    "trunk_link",
    "primary",
    "primary_link",
    "secondary",
    "secondary_link",
    "tertiary",
    "tertiary_link",
    "unclassified",
    "residential",
    "living_street",
    "service"
}

# this maps the road type to an interger to call the c enum
ROAD_TYPE_MAP = {
    "motorway": 0,
    "motorway_link": 1,
    "trunk": 2,
    "trunk_link": 3,
    "primary": 4,
    "primary_link": 5,
    "secondary": 6,
    "secondary_link": 7,
    "tertiary": 8,
    "tertiary_link": 9,
    "unclassified": 10,
    "residential": 11,
    "living_street": 12,
    "service": 13
}

# this calcualtes the real distance
def haversine(lat1, lon1, lat2, lon2):
    R = 6371000  # earth radius in metres
    phi1, phi2 = math.radians(lat1), math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlambda = math.radians(lon2 - lon1)
    a = math.sin(dphi/2)**2 + math.cos(phi1) * math.cos(phi2) * math.sin(dlambda/2)**2
    return R * 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

# OSMIUM runs this for every single node 
class OSMhandler(osmium.SimpleHandler):
    # initialise the handler
    def __init__(self):
        super().__init__()

        # initialise nodes and edges for this object
        self.nodes = {}
        self.edges = []
    
    # creates a node for each datapoint, where its location in the array is its ID
    # stores as a tuple of lat and lon
    def node(self, n):
        self.nodes[n.id] = (n.location.lat, n.location.lon)

    def way(self, w):
        # all roads are called highways
        highway = w.tags.get("highway")

        if highway not in ROAD_TYPES:
            return
        
        # convert the road type string to its matching integer from the C enum, if no type set to 13 (service)
        road_type = ROAD_TYPE_MAP.get(highway, 13)
        
        # set boolean for one way
        one_way = 1 if w.tags.get("oneway") == "yes" else 0

        # converts kmph and mph into integers
        # re gets the first digits found then converts to int
        # if any of it fails convert back to 100kmph
        try:
            raw = w.tags.get("maxspeed", "50")
            if "mph" in raw:
                speed_limit = int(int(re.search(r'\d+', raw).group()) * 1.609)
            else:
                speed_limit = int(re.search(r'\d+', raw).group())
        except (ValueError, AttributeError):
            speed_limit = 100

        # a way is an ordered list of node ids representing the road
        node_ids = [n.ref for n in w.nodes]

        # loop through consecutive pairs of nodes to create edges
        # for example: nodes [A, B, C] creates edges A->B and B->C 
        for i in range(len(node_ids) - 1):
            src = node_ids[i]
            dst = node_ids[i + 1]

            # checks if the src and dst are in the nodes
            if src not in self.nodes or dst not in self.nodes:
                continue
            
            # gets the lat and lon from the src and dst node in the nodes list
            lat1, lon1 = self.nodes[src]
            lat2, lon2 = self.nodes[dst]

            # calculate real distance between the two nodes in kms
            weight = haversine(lat1, lon1, lat2, lon2)

            self.edges.append((src, dst, highway, weight, one_way, speed_limit))
            print(f"src: {src}, dst: {dst}, roadtype: {road_type}, weight: {weight}, one_way?: {one_way}, speed_limit: {speed_limit}")

            # add reverse edge if road is two way
            if not one_way:
                self.edges.append((dst, src, road_type, weight, one_way, speed_limit))

if __name__ == "__main__":
    # if there are more variables then 3 cancel
    if len(sys.argv) != 3:
        print("usage: python scripts/import_osm.py <input.osm.pbf> <output.bin>")
        sys.exit(1)

    input_path = sys.argz[1]
    output_path = sys.argz[2]

    print("parsing osm data...")
    handler = OSMhandler()

    handler.apply_file(input_path, locations=True)

    print(f"nodes: {len(handler.nodes)}, edges: {len(handler.edges)}")