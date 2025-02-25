#!/usr/bin/env python3
import sys
import re
import signal

# YOUR CODE GOES HERE

signal.signal(signal.SIGINT, signal.SIG_IGN)

class Street:
    def __init__(self, name):
        self.name = name
        self.points = []

    def add_point(self, s):
        # detect if the input bracket is correct
        open_count = 0
        for char in s:
            if char == '(':
                open_count = open_count + 1
            elif char == ')':
                open_count = open_count - 1
            if open_count < 0:
                raise Exception('Incorrect input format')
        if open_count != 0:
            raise Exception('Incorrect input format')

        pattern = r'\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)'
        matches = re.findall(pattern, s)
        # detect one point street
        if len(matches) == 1:
            raise Exception('Not a valid street, only one point')
        # detect zero-length line segments
        for i in range(len(matches) - 1):
            if matches[i] == matches[i + 1]:
                raise Exception('Not a valid street, contains zero-length line segments')

        self.points.extend((int(x), int(y)) for x, y in matches)

    def get_points(self):
        return self.points

    def get_name(self):
        return self.name

class Area:
    def __init__(self):
        self.streets = {}

    def add_street(self, street):
        self.streets[street.get_name()] = street

    def get_streets(self):
        return self.streets

    def search_street(self, name):
        if name in self.streets:
            return self.streets[name]
        else:
            return None

    def del_street(self, name):
        if name in self.streets:
            del self.streets[name]
        else:
            raise Exception('this street does not exist')
    
    def print_streets(self):
        for street_name, street in self.streets.items():
            print(f"{street_name}: {street.get_points()}")

    def generate_graph(self):
        vindex = 1;
        V = {}
        E = set()

        for street_name, street in self.streets.items():
            points = street.get_points()
            for i in range(len(points) - 1):
                a1 = points[i]
                a2 = points[i + 1]
                total_intersections = []
                for another_street_name, another_street in self.streets.items():
                    if street != another_street:
                        another_points = another_street.get_points()
                        for j in range(len(another_points) - 1):
                            b1 = another_points[j]
                            b2 = another_points[j + 1]
                            intersect, intersections = self.find_intersection(a1, a2, b1, b2)
                            if intersections != None: total_intersections.extend(intersections)
                            
                # if this segment has intersections, make it into V and E
                if total_intersections:
                    total_intersections.append(a1)
                    total_intersections.append(a2)
                    distinct_points = list(set(total_intersections))
                    sorted_points = sorted(distinct_points, key=lambda p: (p[0], p[1]))
                    # delete intersections that don't belong to this segment
                    a = 0
                    b = 0
                    for i in range(len(sorted_points)):
                        if (sorted_points[i] == a1): a = i
                        if (sorted_points[i] == a2): b = i
                    if (a > b):
                        mid = a
                        a = b
                        b = mid
                    sorted_points = sorted_points[a: b + 1]
                    # add graph
                    for i in range(len(sorted_points) - 1):
                        key1 = 0
                        key2 = 0
                        value1 = sorted_points[i]
                        value2 = sorted_points[i + 1]
                        # if this point already exists
                        for k, v in V.items():
                            if v == value1:
                                key1 = k
                            if v == value2:
                                key2 = k
                        if (key1 == 0):
                            key1 = vindex
                            V[key1] = value1
                            vindex = vindex + 1
                        if (key2 == 0):
                            key2 = vindex
                            V[key2] = value2
                            vindex = vindex + 1
                        # add these two points to E and V
                        E.add((key1,key2))

        return V, E

    def find_intersection(self, a1, a2, b1, b2):
        o1 = self.orientation(a1, a2, b1)
        o2 = self.orientation(a1, a2, b2)
        o3 = self.orientation(b1, b2, a1)
        o4 = self.orientation(b1, b2, a2)

        if o1 != o2 and o3 != o4:
            return True, [self.cal_intersection(a1, a2, b1, b2)]

        intersections = []

        if o1 == 0 and self.on_segment(a1, b1, a2):
            intersections.append((round(b1[0], 2), round(b1[1], 2)))
        if o2 == 0 and self.on_segment(a1, b2, a2):
            intersections.append((round(b2[0], 2), round(b2[1], 2)))
        if o3 == 0 and self.on_segment(b1, a1, b2):
            intersections.append((round(a1[0], 2), round(a1[1], 2)))
        if o4 == 0 and self.on_segment(b1, a2, b2):
            intersections.append((round(a2[0], 2), round(a2[1], 2)))

        if len(intersections) > 0:
            return True, intersections

        return False, None

    def cal_intersection(self, a1, a2, b1, b2):
        x1, y1 = a1
        x2, y2 = a2
        x3, y3 = b1
        x4, y4 = b2

        denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
        if denom == 0:
            return None

        x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom
        y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom

        x_rounded = round(x, 2)
        y_rounded = round(y, 2)

        # avoid (-0.0, -0.0)
        if x_rounded == 0:
            x_rounded = 0.0
        if y_rounded == 0:
            y_rounded = 0.0

        return (x_rounded, y_rounded)

    def orientation(self, a, b, c):
        val = (b[1] - a[1]) * (c[0] - b[0]) - (b[0] - a[0]) * (c[1] - b[1])
        if val == 0:
            return 0
        elif val > 0:
            return 1
        else:
            return 2

    def on_segment(self, a, b, c):
        val = (b[0] <= max(a[0], c[0]) and b[0] >= min(a[0], c[0]) and
            b[1] <= max(a[1], c[1]) and b[1] >= min(a[1], c[1]))
        return val
        

def parseLine(line):
    sp = line.strip()
    sp = sp.split('"', 2)
    sp = [s.strip() for s in sp]
    if (len(sp) > 3):
        raise Exception('wrong input')
    cmd = sp[0]
    val1 = None
    val2 = None
    if cmd == 'gg':
        if len(sp) > 1:
            raise Exception('too many arguments')
    elif (cmd == 'rm'):
        if len(sp) < 2:
            raise Exception('too few arguments')
        val1 = sp[1]
    elif (cmd == 'add' or cmd == 'mod'):
        if len(sp) < 3:
            raise Exception('too few arguments')
        val1 = sp[1]
        val2 = sp[2]
    else:
        raise Exception('unknown command: ' + cmd)
    return cmd, val1, val2



def main():
    # YOUR MAIN CODE GOES HERE

    # sample code to read from stdin.
    # make sure to remove all spurious print statements as required
    # by the assignment
    area = Area()


    while True:
        line = sys.stdin.readline()
        if line == '':
            break
        try:
            cmd, val1, val2 = parseLine(line)

            if cmd == 'add':
                street_name = val1.lower()
                street = Street(street_name)

                # add points of this street (use regular expression)
                street.add_point(val2)

                # search if this street already exists
                old_street = area.search_street(street_name)
                if old_street != None:
                    raise Exception('this street already exists')
                # add this street to area
                area.add_street(street)
            elif cmd == 'rm':
                street_name = val1.lower()
                area.del_street(street_name)
            elif cmd == 'mod':
                street_name = val1.lower()
                old_street = area.search_street(street_name)
                if old_street == None:
                    raise Exception('this street does not exist')
                # build new street
                new_street = Street(street_name)
                new_street.add_point(val2)
                area.del_street(street_name)
                area.add_street(new_street)
            elif cmd == 'gg':
                V, E = area.generate_graph()
                # output V for a2
                print(f"V {len(V)}", file = sys.stdout)

                # output E for a2
                edges = ",".join(f"<{edge[0]},{edge[1]}>" for edge in E)
                V_output = f"E {{{edges}}}"
                print(V_output, file = sys.stdout)
            else:
                raise Exception('unknown command: ' + cmd)

            sys.stdout.flush();
        except Exception as e:
            print('Error: ' + str(e), file=sys.stderr)

    sys.exit(0)


if __name__ == "__main__":
    main()
