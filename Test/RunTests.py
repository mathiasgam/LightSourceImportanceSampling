import subprocess
import os

import numpy as np
import matplotlib.pyplot as plt


# instance = subprocess.run(["../bin/Release-windows-x86_64/PathTracer.exe", "-obj ../Assets/Models/Helix.obj -obj ../Assets/Models/Buddha.obj -obj ../Assets/Models/Background.obj"])

# Change working directory to the PathTracer
os.chdir("c:/repos/LightSourceImportanceSampling/PathTracer")


#arguments = ["-n","100","-out", "../Test/Output/","-name",  "TestRun"]

#scene = ["-obj","../Assets/Models/Helix.obj", "-obj", "../Assets/Models/Buddha.obj", "-obj", "../Assets/Models/Background.obj"]
#camera = ["-pos", "-0.0", "1.0", "2.72", "-rot", "-0.0", "-0.0", "0.0"]

#subprocess.run(["../bin/Release-windows-x86_64/PathTracer.exe"] + scene + arguments)

def run(args):
    subprocess.run(["../bin/Release-windows-x86_64/PathTracer.exe"] + args)

def arg_num_samples(n:int):
    return ["-n", str(n)]

def arg_load_obj(filepath:str):
    return ["-obj", filepath]

def arg_scene(files:list):
    res = []
    for file in files:
        res = res + arg_load_obj(file)
    return res

def arg_cam_pos(x:float, y:float, z:float):
    return ["-pos", str(x), str(y), str(z)]

def arg_cam_rot(x:float, y:float, z:float):
    return ["-rot", str(x), str(y), str(z)]

def arg_output(filepath:str):
    return ["-out", filepath]


models = ["../Assets/Models/Helix.obj", "../Assets/Models/CornellBox.obj"]

args = arg_scene(models) + arg_num_samples(1000)

run(args)