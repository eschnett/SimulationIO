import pysimulationio.H5 as H5
import pysimulationio.RegionCalculus as RegionCalculus
import pysimulationio.SimulationIO as SimulationIO
from pysimulationio.SimulationIO import createProject

# TODO: Make a wrapper class around project
# with open,close and enter methods for easier syntax.
def readProject(filename):
    """Given the name of a SimulationIO file, reads in the project object
    and returns it. Also returns the H5 Handle for the project object,
    which can be closed later.

    returns: project, file_handle"""
    f = H5.H5File(filename,
                  H5.H5F_ACC_RDONLY,
                  H5.FileCreatPropList(),
                  H5.FileAccPropList())
    project = SimulationIO.readProject(f)
    return project,f

def writeProject(project,filename):
    """Given a project object and a SimulationIO file, writes the project
    to the file. Returns the H5 handle for the project object, which
    can be closed later."""
    f = H5.H5File(filename,
                  H5.H5F_ACC_TRUNC,
                  H5.FileCreatPropList(),
                  H5.FileAccPropList())
    project.write(f)
    return f
