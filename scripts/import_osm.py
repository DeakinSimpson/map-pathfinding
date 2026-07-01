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

# gets all the ways that are roads
class PassOneHandler(osmium.SimpleHandler):
    def __init__(self):
        super().__init__()
        self.road_node_ids = set()

    def way(self, w):
        if w.tags.get("highway") not in ROAD_TYPES:
            return
        for n in w.nodes:
            self.road_node_ids.add(n.ref)

# gets all the nodes and ways that are a part of roads
class OSMhandler(osmium.SimpleHandler):
    # initialise the handler
    def __init__(self, road_node_ids):
        super().__init__()
        self.road_node_ids = road_node_ids

        # initialise nodes and edges for this object
        self.nodes = {}
        self.edges = []

    # creates a node for each datapoint, where its location in the array is its ID
    # stores as a tuple of lat and lon
    # only stores nodes that are referenced by road ways
    def node(self, n):
        if n.id in self.road_node_ids:
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

            self.edges.append((src, dst, road_type, weight, one_way, speed_limit))

            # add reverse edge if road is two way
            if not one_way:
                self.edges.append((dst, src, road_type, weight, one_way, speed_limit))

def write_binary(handler, output_path):
    # open output path with wb (write binary)
    with open(output_path, "wb") as f:
        # writes to f
        # struct.pack converts to bytes
        # write q type (long long) of number of nodes
        f.write(struct.pack("q", len(handler.nodes)))

        # write number of edges
        f.write(struct.pack("q", len(handler.edges)))

        # write each node 
        # write node id ad long long (q)
        # write lat and lon as double (d)
        for node_id, (lat, lon) in handler.nodes.items():
            f.write(struct.pack("qdd", node_id, lat, lon))

        # write each edge
        for src, dst, road_type, weight, one_way, speed_limit in handler.edges:
            f.write(struct.pack("qqdiiii", src, dst, weight, road_type, one_way, speed_limit, 0))


if __name__ == "__main__":
    # if there are more variables then 3 cancel
    if len(sys.argv) != 3:
        print("usage: python scripts/import_osm.py <input.osm.pbf> <output.bin>")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]

    print("pass 1: collecting road node ids...")
    pass1 = PassOneHandler()
    pass1.apply_file(input_path)
    print(f"road nodes found: {len(pass1.road_node_ids)}")

    print("pass 2: parsing osm data...")
    handler = OSMhandler(pass1.road_node_ids)
    handler.apply_file(input_path, locations=True)
    print(f"nodes: {len(handler.nodes)}, edges: {len(handler.edges)}")

    print("writing to binary")
    write_binary(handler, output_path)
    print("done")