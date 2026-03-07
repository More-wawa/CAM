#pragma once

#ifndef CAM_VTKMANAGER_H
#define CAM_VTKMANAGER_H

#include "EnumType.h"
#include <vtkSmartPointer.h>

class QString;
class QVTKOpenGLNativeWidget;
class vtkRenderer;
class vtkRenderWindow;
class vtkInteractorStyleTrackballCamera;
class vtkCompositePolyDataMapper;
class vtkOCCTReader;
class vtkActor;
class vtkCamera;
class vtkAxesActor;
class vtkOrientationMarkerWidget;

class VTKManager {
public:
    static VTKManager* New();

    [[nodiscard]] QVTKOpenGLNativeWidget * m_vtk_widget() const {
        return m_vtkWidget;
    }

    void init();
    ErrorType openModelFile(QString fileName);
    void setStandardView(double dx, double dy, double dz, double ux, double uy, double uz) const;

private:
    bool initialized = false;

    QVTKOpenGLNativeWidget *m_vtkWidget = nullptr;
    vtkSmartPointer<vtkRenderer> m_vtkRenderer;
    vtkSmartPointer<vtkRenderWindow> m_vtkRenderWindow;
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> m_vtkStyle;
    vtkSmartPointer<vtkCompositePolyDataMapper> m_vtkMapper;
    vtkSmartPointer<vtkOCCTReader> m_vtkOCCTReader;
    vtkSmartPointer<vtkActor> m_vtkActor;
    vtkSmartPointer<vtkCamera> m_vtkCamera;
    vtkSmartPointer<vtkAxesActor> m_vtkAxesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_vtkOrientationMarkerWidget;
};


#endif //CAM_VTKMANAGER_H
