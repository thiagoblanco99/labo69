import numpy as np
import matplotlib.pyplot as plt

# Parameters of the waveguide
a = 0.01 # width of the waveguide in meters
b = 0.02 # height of the waveguide in meters
L = 0.1 # length of the waveguide in meters
dx = 0.001 # spatial step size
dt = dx/np.sqrt(2) # time step size
tmax = 0.1 # maximum simulation time

# Constants
mu0 = 4*np.pi*1e-7 # vacuum permeability
eps0 = 8.8541878128e-12 # vacuum permittivity
c = 1/np.sqrt(mu0*eps0) # speed of light in vacuum

# Grid parameters
Nx = int(L/dx) # number of grid points in x
Ny = int(b/dx) # number of grid points in y
Nt = int(tmax/dt) # number of time steps

# Initialize the grid
Ex = np.zeros((Nx+1, Ny)) # electric field in x-direction
Ey = np.zeros((Nx, Ny+1)) # electric field in y-direction
Hz = np.zeros((Nx, Ny)) # magnetic field in z-direction

# Set the initial condition
x0 = L/2
y0 = b/2
sigma = 0.01
Ex = np.exp(-((np.linspace(0, L, Nx+1)-x0)/sigma)**2)[:, np.newaxis]
Ey = np.zeros((Nx, Ny+1))

# Main simulation loop
for n in range(Nt):
    # Update the electric fields
    for i in range(1, Nx):
        for j in range(1, Ny+1):
            Ex[i,j-1] = Ex[i,j-1] + (dt/(eps0*dx))*(Hz[i,j-1]-Hz[i-1,j-1])
    for i in range(1, Nx+1):
        for j in range(1, Ny):
            Ey[i-1,j] = Ey[i-1,j] - (dt/(eps0*dx))*(Hz[i-1,j]-Hz[i-1,j-1])

    # Update the magnetic field
    for i in range(1, Nx):
        for j in range(1, Ny):
            Hz[i,j] = Hz[i,j] + (dt/(mu0*dx))*(Ey[i,j]-Ey[i-1,j]-Ex[i,j]+Ex[i,j-1])

    # Apply boundary conditions
    Ex[0,:] = 0
    Ex[-1,:] = 0
    Ex[:,0] = 0
    Ex[:,-1] = 0
    Ey[0,:] = 0
    Ey[-1,:] = 0
    Ey[:,0] = 0
    Ey[:,-1] = 0

# Plot the electric field
plt.imshow(np.sqrt(Ex**2+Ey**2).T, cmap='jet', origin='lower', extent=[0, L, 0, b])
plt.colorbar()
plt.xlabel('x (m)')
plt.ylabel('y (m)')
plt.title('Electric field in a rectangular waveguide')
plt.show()
