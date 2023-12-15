import ctypes
import pathlib
import time
import os
import sys
from tkinter import *
from ctypes import *
import random
import time


##################################################
# Initialize C bindings
##################################################

# Load the shared library into ctypes
if sys.platform == 'win32':
  libname = os.path.join(pathlib.Path().absolute(), "libparticle.dll")
else : 
  libname = os.path.join(pathlib.Path().absolute(), "libparticle.so")
c_lib = ctypes.CDLL(libname)

# Define python equivalent class

class PARTICLE(Structure):
    _fields_ = [("position", c_float*2),
                ("next_pos", c_float*2),
                ("velocity", c_float*2),
                ("inv_mass", c_float),
                ("radius",   c_float),
                ("solid_id", c_int),
                ("draw_id",  c_int),
                ("halo_id", c_int),
                ("temperature", c_float)]
    
class SPHERECOLLIDER(Structure):
    _fields_ = [("center", c_float*2), 
              ("radius", c_float)]
    
class PLANECOLLIDER(Structure):
    _field_ = [("point", c_float*2),
               ("normale", c_float*2)]

class CONTEXT(Structure):
    _fields_ = [("num_particles", c_int),
                ("capacity_particles", c_int),
                ("particles", POINTER(PARTICLE) ),
                ("num_ground_sphere", c_int),
                ("ground_spheres", POINTER(SPHERECOLLIDER)), 
                ("num_ground_plane", c_int), 
                ("ground_plane", POINTER(PLANECOLLIDER))]

# ("pos", c_float*2) => fixed size array of two float


# Declare proper return types for methods (otherwise considered as c_int)
c_lib.initializeContext.restype = POINTER(CONTEXT) # return type of initializeContext is Context*
c_lib.getParticle.restype = PARTICLE
c_lib.getGroundSphereCollider.restype = SPHERECOLLIDER
# WARNING : python parameter should be explicitly converted to proper c_type of not integer.
# If we already have a c_type (including the one deriving from Structure above)
# then the parameter can be passed as is.

##################################################
# Class managing the UI
##################################################

def color_generator():
    r, g, b = random.randint(0,255), random.randint(0,255),random.randint(0,255)
    return f"#{r:02x}{g:02x}{b:02x}"

def couleur_orange(t):
    r = int(min(t*510, 255))
    g = int(t*255)
    b = int(max(0, (t-0.5)*510))
    return f"#{r:02x}{g:02x}{b:02x}"

def couleur_rouge(t):
    r = int(min(t*510, 255))
    g = 0
    b = 0
    return f"#{r:02x}{g:02x}{b:02x}"

def couleur_chaleur(t):
    if t>0.5:
        return couleur_orange(t)
    else:
        return couleur_rouge(t)
    
def couleur_chaleur2(t):
    """on affiche uniquement celle sufisament chaude"""
    if t>0.47:
       return couleur_chaleur(t)
    else:
        return couleur_chaleur(0)
    
def couleur_chaleurRVBA(t):
    """pour le halo lumnineux"""
    alpha = 0.5
    couleur = couleur_chaleur2(t)
    couleur_rvba = couleur + hex(int(alpha * 255))[2:].zfill(2)
    return couleur_rvba
    
#_______________________________________________________
 
class ParticleUI :
    def __init__(self) :
        # create drawing context
        self.context = c_lib.initializeContext(2000)
        self.width = 1200
        self.height = 800

        # physical simulation will work in [-world_x_max,world_x_max] x [-world_y_max,world_y_max]
        self.world_x_max = 15
        self.world_y_max = 10 
        # WARNING : the mappings assume world bounding box and canvas have the same ratio !

        self.window = Tk()

        # create simulation context...
        self.canvas = Canvas(self.window,width=self.width,height=self.height)
        self.canvas.grid()
        
        self.matrice_bloom = [[0 for _ in range(2000)] for _ in range(2000)]

        # Initialize drawing, only needed if the context is initialized with elements,
        # otherwise, see addParticle
        
        for i in range(self.context.contents.num_ground_sphere):
            g_sphere = c_lib.getGroundSphereCollider(self.context, i)
            x, y = g_sphere.center
            r = g_sphere.radius
            x1, y1 = self.worldToView((x-r, y-r))
            x2, y2 = self.worldToView((x+r, y+r))
            
            self.canvas.create_oval(x1, y1, x2, y2, fill="blue")
        
        #self.canvas.create_oval(400-50, 400-50, 400+50, 400+50, fill="yellow")
        
        
        for i in range(self.context.contents.num_particles):
            sphere = c_lib.getParticle(self.context, i)
            draw_id = self.canvas.create_oval(*self.worldToView( (sphere.position[0]-sphere.radius,sphere.position[1]-sphere.radius) ),
                                              *self.worldToView( (sphere.position[0]-sphere.radius,sphere.position[1]-sphere.radius) ),
                                              fill="orange")
            c_lib.setDrawId(self.context, i, draw_id)
        
        # Initialize Mouse and Key events
        self.canvas.bind("<Button-1>", lambda event: self.mouseCallback(event))
        self.window.bind("<Key>", lambda event: self.keyCallback(event)) # bind all key
        self.window.bind("<Escape>", lambda event: self.enterCallback(event))
        self.window.bind("<e>", lambda event: self.demarage(event))
        # bind specific key overide default binding

    def launchSimulation(self) :
        # launch animation loop
        self.animate()
        # launch UI event loop
        self.window.mainloop()



    def animate(self) :
        """ animation loop """
        # APPLY PHYSICAL UPDATES HERE !
        for i in range(6) :
            c_lib.updatePhysicalSystem(self.context, c_float(0.05/float(6)), 1) #TODO can be called more than once..., just devise dt
            
        for i in range(self.context.contents.num_particles):
            sphere = c_lib.getParticle(self.context, i)
            #### le 1.2*sphere est a enlever pour voir les sphère, on les traces plus grosse pour l'effet visuel seullement
            #print(sphere.position[0],sphere.position[1])
            self.canvas.coords(sphere.draw_id,
                               *self.worldToView( (sphere.position[0]-1*sphere.radius,sphere.position[1]-1*sphere.radius) ),
                               *self.worldToView( (sphere.position[0]+1*sphere.radius,sphere.position[1]+1*sphere.radius) ) )
            #self.canvas.coords(sphere.halo_id,
            #                   *self.worldToView( (sphere.position[0]-1.3*sphere.radius,sphere.position[1]-1.3*sphere.radius) ),
            #                   *self.worldToView( (sphere.position[0]+1.3*sphere.radius,sphere.position[1]+1.3*sphere.radius) ) )
            
            #self.canvas.itemconfig(sphere.halo_id, fill=couleur_chaleurRVBA(sphere.temperature), outline=couleur_chaleurRVBA(sphere.temperature))
            self.canvas.itemconfig(sphere.draw_id, fill=couleur_chaleur(sphere.temperature), outline=couleur_chaleur(sphere.temperature))
        """   tentative de bloom
        for i in range(self.context.contents.num_particles):
            for j in range(self.context.contents.num_particles):
                sphere1 = c_lib.getParticle(self.context, i)
                sphere2 = c_lib.getParticle(self.context, j)
                x1, y1 = sphere1.position
                x2, y2 = sphere2.position
                radius1, radius2 = 1.3*sphere1.radius, 1.3*sphere2.radius
                if abs(x2 - x1) <= radius1+radius2 and abs(y2 - y1) <= radius1+radius2:
                    intersection_color = couleur_chaleur2(0.5*sphere1.temperature+0.5*sphere2.temperature)
                    x_left = max(x1 - radius1, x2 - radius2)
                    y_top = max(y1 - radius1, y2 - radius2)
                    x_right = min(x1 + radius1, x2 + radius2)
                    y_bottom = min(y1 + radius1, y2 + radius2)
                    self.canvas.delete(self.matrice_bloom[i][j])
                    self.matrice_bloom[i][j] = self.canvas.create_oval(x_left, y_top, x_right, y_bottom, fill=intersection_color)
        
        """         
        self.window.update()
        self.window.after(5, self.animate)

    # Conversion from worl space (simulation) to display space (tkinter canvas)
    def worldToView(self, world_pos) :
        return ( self.width *(0.5 + (world_pos[0]/self.world_x_max) * 0.5),
                 self.height *(0.5 - (world_pos[1]/self.world_y_max) * 0.5)) 
    
    
    def viewToWorld(self, view_pos) :
        return ( self.world_x_max * 2.0 * (view_pos[0]/self.width - 0.5) ,
                 self.world_y_max * 2.0 * (0.5-view_pos[1]/self.height))  

    def addParticle(self, screen_pos, radius, mass) :
        (x_world, y_world) = self.viewToWorld(screen_pos)
        # min max bounding box in view coordinates, will be propertly initialized 
        # in the canvas oval after the first call to animate
        #b_min = self.worldToView( (x_world-radius,y_world-radius) )
        #b_max = self.worldToView( (x_world+radius,y_world+radius) )
        draw_id = self.canvas.create_oval(0,0,0,0,fill='black')
        halo_id = self.canvas.create_oval(0, 0, 0, 0, fill ='black')
        c_lib.addParticle(self.context, 
                        c_float(x_world), c_float(y_world), 
                        c_float(radius), c_float(mass),
                        draw_id, halo_id)

    # All mouse and key callbacks

    def mouseCallback(self, event):
        self.addParticle((event.x,event.y), 0.2, 1.0)
    
    def keyCallback(self, event):
        print(repr(event.char))
    def enterCallback(self, event):
        self.window.destroy()
    
    def demarage(self, e):
        x, y = 100, 100
        for i in range(800): #vrai phénomène de convection a partir de 900 (1000 très bon mais lent)
            self.addParticle((int(x+3*random.random()), int(y+3*random.random())), 0.15, 1)
            if x == 1100:
                x = 100
                y += 20
            else:
                x += 20
        


gui = ParticleUI()
gui.launchSimulation()

