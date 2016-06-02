import H5
import RegionCalculus
import SimulationIO
from SimulationIO import createProject

def readProject(filename):
    f = H5.H5File(filename,
                  H5.H5F_ACC_RDONLY,
                  H5.FileCreatPropList(),
                  H5.FileAccPropList())
    project = SimulationIO.readProject(f)
    return project

def writeProject(project,filename):
    f = H5.H5File(filename,
                  H5.H5_ACC_TRUNC,
                  H5.FileCreatPropList(),
                  H5.FileAccPropList())
    project.write(f)
    f.close()

