#!/usr/bin/env python3
from samplebase import SampleBase
import socket
import sys
import time
import numpy as np
import json

class HeatMap(SampleBase):
    def __init__(self, *args, **kwargs):
        super(HeatMap, self).__init__(*args, **kwargs)
        self.x = []
        self.y = []
        self.z = []
        self.id = []
        self.delete_life = []
        self.color_map = {}
        self.colors = [(255, 0, 0), 
                (0, 255, 0),
                (0, 0, 255),
                (255, 255, 0),
                (0, 255, 255),
                (255, 0, 255)]
        self.max_meters = 1

    def run(self):
        self.width = self.matrix.width
        self.height = self.matrix.height
        print("LEDs Running")
        
#        sub_blocks = 16V
#        width = self.matrix.width
#        height = self.matrix.height
#        x_step = max(1, width / sub_blocks)
#        y_step = max(1, height / sub_blocks)
#        count = 0
#
#        while True:
#            for y in range(0, height):
#                for x in range(0, width):
#                    c = sub_blocks * int(y / y_step) + int(x / x_step)
#                    if count % 4 == 0:
#                        self.matrix.SetPixel(x, y, c, c, c)
#                    elif count % 4 == 1:
#                        self.matrix.SetPixel(x, y, c, 0, 0)
#                    elif count % 4 == 2:
#                        self.matrix.SetPixel(x, y, 0, c, 0)
#                    elif count % 4 == 3:
#                        self.matrix.SetPixel(x, y, 0, 0, c)
#
#            count += 1
#       time.sleep(2)


def draw_point(hmap, idx):
    print(f"Drawing point {idx}")
    x_point = np.argmin(np.abs(hmap.x[idx] - hmap.x_grid))
    y_point = np.argmin(np.abs(hmap.y[idx] - hmap.y_grid))
    id_point = hmap.id[idx]
    for dx in range(-1,2):
        for dy in range(-1,2):
            if x_point+dx < hmap.width and x_point+dx >= 0 and y_point+dx < hmap.width and y_point+dx >= 0: 
                hmap.matrix.SetPixel(x_point+dx, y_point+dy, hmap.color_map[id_point][0], \
                    hmap.color_map[id_point][1], hmap.color_map[id_point][2])

def draw(hmap):
    print("Drawing all")
    for x in range(0, hmap.width):
        for y in range(0, hmap.height):
            if 29 < x < 35 and 29 < y < 35:
                hmap.matrix.SetPixel(x, y, 255, 255, 255)
            else:
                hmap.matrix.SetPixel(x, y, 0, 0, 0)

    for i in range(len(hmap.id)):
        draw_point(hmap, i)

# Main function
if __name__ == "__main__":
    my_ip = '192.168.1.209'
    hmap = HeatMap()
    if not hmap.process():
        hmap.print_help()
    hmap.x_grid = np.linspace(-hmap.max_meters, hmap.max_meters, hmap.matrix.width)
    hmap.y_grid = np.linspace(-hmap.max_meters, hmap.max_meters, hmap.matrix.height)
    print(hmap.x_grid)
    print(hmap.y_grid)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((my_ip, 30000))
    sock.listen(5)
    (conn, addr) = sock.accept()
    print(f"{conn} | {addr}")
    while True:
        data = conn.recv(80)
        data = data.decode("utf-8")
        data = data.rstrip("\x00")
        print(data)
        ids = []
        xs = []
        ys = []
        zs = []
        if data[0] != '$':
            print(data[0])
            continue
        tags = data[1:].strip().split('|')
        if '' in tags: tags.remove('')
        print(tags)
        for tag in tags:
            print(f"\t Doing Tag {tag}")
            fields = tag.strip().split(',')
            if '' in fields: fields.remove('')
            print("\t {fields}")
            if len(fields) == 0:
                continue
            for field in fields:
                print(f"\t\t Doing Field {field}")
                key,val = field.strip().split(':')
                if key == 'x':
                    xs.append(float(val))
                if key == 'z':
                    ys.append(float(val))
                if key == 'y':
                    zs.append(float(val))
                if key == 'id':
                    ids.append(float(val))
        print(f"IDs: {ids}")
        print(f"Xs:  {xs}")
        print(f"Ys:  {ys}")
        print(f"Zs:  {zs}")
        ids_to_remove = []
        for i in range(len(hmap.id)):
            if hmap.id[i] not in ids:
                hmap.delete_life[i] -= 1
                if hmap.delete_life[i] <= 0:
                    # need to remove apriltag that we no longer see
                    print(f"Removing ID {hmap.id[i]}")
                    ids_to_remove.append(hmap.id[i])
                    del hmap.color_map[hmap.id[i]]
            else:
                # need to update x,y,z coordinate of existing april tag
                print(f"Updating ID {hmap.id[i]}")
                idx = ids.index(hmap.id[i])
                hmap.x[i] = xs[idx]
                hmap.y[i] = ys[idx]
                hmap.z[i] = zs[idx]
                hmap.delete_life[i] = 50
        for i in range(len(ids_to_remove)):
            print(f"Deleting ID {ids_to_remove[i]}")
            idx = hmap.id.index(ids_to_remove[i])
            del hmap.id[idx]
            del hmap.x[idx]
            del hmap.y[idx]
            del hmap.z[idx]
            del hmap.delete_life[idx]
        for new_id in (set(ids) - set(hmap.id)):
            print(f"Adding new ID {new_id}")
            idx = ids.index(new_id)
            hmap.id.append(new_id)
            hmap.x.append(xs[idx])
            hmap.y.append(ys[idx])
            hmap.z.append(zs[idx])
            hmap.delete_life.append(50)
            for color in hmap.colors:
                if color not in hmap.color_map.values():
                    print(f"\t Getting color {color}")
                    hmap.color_map[new_id] = color
                    break
        draw(hmap)

    
