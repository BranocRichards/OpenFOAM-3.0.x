/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2013 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "pqPV3FoamReaderPanel.h"

// QT
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QString>
#include <QPushButton>
#include <QtDebug>

// Paraview <-> QT UI
#include "pqAnimationScene.h"
#include "pqApplicationCore.h"
#include "pqPipelineRepresentation.h"
#include "pqServerManagerModel.h"
#include "pqView.h"

// Paraview Server Manager
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMProperty.h"
#include "vtkSMSourceProxy.h"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

pqPV3FoamReaderPanel::pqPV3FoamReaderPanel
(
    pqProxy *proxy,
    QWidget *p
)
:
    pqAutoGeneratedObjectPanel(proxy, p)
{
    // create first sublayout (at top of the panel)
    QGridLayout* form = new QGridLayout();
    this->PanelLayout->addLayout(form, 0, 0, 1, -1);

    // ROW 0
    // ~~~~~

    vtkSMProperty* prop = 0;

    // refresh button for updating times/fields
    if ((prop = this->proxy()->GetProperty("UiRefresh")) != 0)
    {
        prop->SetImmediateUpdate(1);
        QPushButton *refresh = new QPushButton("Refresh Times");
        refresh->setToolTip("Rescan for updated times/fields.");

        form->addWidget(refresh, 0, 0, Qt::AlignLeft);
        QObject::connect
        (
            refresh,
            SIGNAL(clicked()),
            this,
            SLOT(RefreshPressed())
        );
    }

    // checkbox for skip zeroTime
    if ((prop = this->proxy()->GetProperty("UiZeroTime")) != 0)
    {
        // immediate update on the Server Manager side
        prop->SetImmediateUpdate(true);

        ZeroTime_ = new QCheckBox("Skip Zero Time");
        ZeroTime_->setChecked
        (
            vtkSMIntVectorProperty::SafeDownCast(prop)->GetElement(0)
        );
        ZeroTime_->setToolTip
        (
            "Skip including the 0/ time directory."
        );

        form->addWidget(ZeroTime_, 0, 1, Qt::AlignLeft);
        connect
        (
            ZeroTime_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(ZeroTimeToggled())
        );
    }

    // ROW 1
    // ~~~~~

    QFrame* hline1 = new QFrame(this);
    hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    form->addWidget(hline1, 1, 0, 1, 3);

    // ROW 2
    // ~~~~~

    // checkbox for caching mesh
    if ((prop = this->proxy()->GetProperty("UiCacheMesh")) != 0)
    {
        // immediate update on the Server Manager side
        prop->SetImmediateUpdate(true);

        CacheMesh_ = new QCheckBox("Cache Mesh");
        CacheMesh_->setChecked
        (
            vtkSMIntVectorProperty::SafeDownCast(prop)->GetElement(0)
        );
        CacheMesh_->setToolTip
        (
            "Cache the fvMesh in memory."
        );

        form->addWidget(CacheMesh_, 2, 0, Qt::AlignLeft);
        connect
        (
            CacheMesh_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(CacheMeshToggled())
        );
    }

    // cell 2,1 empty

    // ROW 3
    // ~~~~~

    // checkbox for include sets
    if ((prop = this->proxy()->GetProperty("UiIncludeSets")) != 0)
    {
        // immediate update on the Server Manager side
        prop->SetImmediateUpdate(true);

        IncludeSets_ = new QCheckBox("Include Sets");
        IncludeSets_->setChecked
        (
            vtkSMIntVectorProperty::SafeDownCast(prop)->GetElement(0)
        );
        IncludeSets_->setToolTip
        (
            "Search the polyMesh/sets/ directory."
        );

        // row/col 1,0
        form->addWidget(IncludeSets_, 3, 0, Qt::AlignLeft);
        connect
        (
            IncludeSets_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(IncludeSetsToggled())
        );
    }

    // checkbox for Groups Only
    if ((prop = this->proxy()->GetProperty("UiShowGroupsOnly")) != 0)
    {
        // immediate update on the Server Manager side
        prop->SetImmediateUpdate(true);

        ShowGroupsOnly_ = new QCheckBox("Groups Only");
        ShowGroupsOnly_->setChecked
        (
            vtkSMIntVectorProperty::SafeDownCast(prop)->GetElement(0)
        );
        ShowGroupsOnly_->setToolTip
        (
            "Show patchGroups only."
        );

        // row/col 2, 2
        form->addWidget(ShowGroupsOnly_, 3, 1, Qt::AlignLeft);
        connect
        (
            ShowGroupsOnly_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(ShowGroupsOnlyToggled())
        );
    }


    // ROW 4
    // ~~~~~

    // checkbox for include zones
    if ((prop = this->proxy()->GetProperty("UiIncludeZones")) != 0)
    {
        // immediate update on the Server Manager side
        prop->SetImmediateUpdate(true);

        IncludeZones_ = new QCheckBox("Include Zones");
        IncludeZones_->setChecked
        (
            vtkSMIntVectorProperty::SafeDownCast(prop)->GetElement(0)
        );
        IncludeZones_->setToolTip
        (
            "ZoneMesh information is used to find {cell,face,point}Zones. "
            "The polyMesh/ directory is only checked on startup."
        );

        // row/col 1,1
        form->addWidget(IncludeZones_, 4, 0, Qt::AlignLeft);
        connect
        (
            IncludeZones_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(IncludeZonesToggled())
        );
    }

    // checkbox for patch names
    if ((prop = this->proxy()->GetProperty("UiShowPatchNames")) != 0)
    {
        // immediate update on the Server Manager side
        prop->SetImmediateUpdate(true);

        ShowPatchNames_ = new QCheckBox("Patch Names");
        ShowPatchNames_->setChecked
        (
            vtkSMIntVectorProperty::SafeDownCast(prop)->GetElement(0)
        );
        ShowPatchNames_->setToolTip
        (
            "Show patch names in render window."
        );

        // row/col 0,1
        form->addWidget(ShowPatchNames_, 4, 1, Qt::AlignLeft);
        connect
        (
            ShowPatchNames_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(ShowPatchNamesToggled())
        );
    }

    // ROW 5
    // ~~~~~

    QFrame* hline2 = new QFrame(this);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    form->addWidget(hline2, 5, 0, 1, 3);

    // ROW 6
    // ~~~~~

    // checkbox for vol field interpolation
    if ((prop = this->proxy()->GetProperty("UiInterpolateVolFields")) != 0)
    {
        // immediate update on the Server Manager side
        prop->SetImmediateUpdate(true);

        InterpolateVolFields_ = new QCheckBox("Interpolate volFields");
        InterpolateVolFields_->setChecked
        (
            vtkSMIntVectorProperty::SafeDownCast(prop)->GetElement(0)
        );
        InterpolateVolFields_->setToolTip
        (
            "Interpolate volFields into pointFields"
        );

        // row/col 1,1
        form->addWidget(InterpolateVolFields_, 6, 0, Qt::AlignLeft);
        connect
        (
            InterpolateVolFields_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(InterpolateVolFieldsToggled())
        );
    }

    // checkbox for extrapolate patches
    if ((prop = this->proxy()->GetProperty("UiExtrapolatePatches")) != 0)
    {
        // immediate update on the Server Manager side
        prop->SetImmediateUpdate(true);

        ExtrapolatePatches_ = new QCheckBox("Extrapolate Patches");
        ExtrapolatePatches_->setChecked
        (
            vtkSMIntVectorProperty::SafeDownCast(prop)->GetElement(0)
        );
        ExtrapolatePatches_->setToolTip
        (
            "Extrapolate internalField to non-constraint patches"
        );

        // row/col 1,1
        form->addWidget(ExtrapolatePatches_, 6, 1, Qt::AlignLeft);
        connect
        (
            ExtrapolatePatches_,
            SIGNAL(stateChanged(int)),
            this,
            SLOT(ExtrapolatePatchesToggled())
        );
    }

    // ROW 7
    // ~~~~~

    QFrame* hline3 = new QFrame(this);
    hline3->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    form->addWidget(hline3, 7, 0, 1, 3);
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void pqPV3FoamReaderPanel::CacheMeshToggled()
{
    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiCacheMesh")
    )->SetElement(0, CacheMesh_->isChecked());
}


void pqPV3FoamReaderPanel::RefreshPressed()
{
    // update everything
    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiRefresh")
    )->Modified();

    vtkSMSourceProxy::SafeDownCast(this->proxy())->UpdatePipeline();

    // render all views
    pqApplicationCore::instance()->render();
}


void pqPV3FoamReaderPanel::ZeroTimeToggled()
{
    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiZeroTime")
    )->SetElement(0, ZeroTime_->isChecked());

    this->setModified();
}


void pqPV3FoamReaderPanel::ShowPatchNamesToggled()
{
    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiShowPatchNames")
    )->SetElement(0, ShowPatchNames_->isChecked());

    // update the active view
    if (this->view())
    {
        this->view()->render();
    }
    // OR: update all views
    // pqApplicationCore::instance()->render();
}


void pqPV3FoamReaderPanel::ShowGroupsOnlyToggled()
{
    vtkSMProperty* prop;

    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiShowGroupsOnly")
    )->SetElement(0, ShowGroupsOnly_->isChecked());

    if ((prop = this->proxy()->GetProperty("PartArrayStatus")) != 0)
    {
        this->proxy()->UpdatePropertyInformation(prop);
    }
}


void pqPV3FoamReaderPanel::IncludeSetsToggled()
{
    vtkSMProperty* prop;

    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiIncludeSets")
    )->SetElement(0, IncludeSets_->isChecked());

    if ((prop = this->proxy()->GetProperty("PartArrayStatus")) != 0)
    {
        this->proxy()->UpdatePropertyInformation(prop);
    }
}


void pqPV3FoamReaderPanel::IncludeZonesToggled()
{
    vtkSMProperty* prop;

    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiIncludeZones")
    )->SetElement(0, IncludeZones_->isChecked());

    if ((prop = this->proxy()->GetProperty("PartArrayStatus")) != 0)
    {
        this->proxy()->UpdatePropertyInformation(prop);
    }
}


void pqPV3FoamReaderPanel::ExtrapolatePatchesToggled()
{
    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiExtrapolatePatches")
    )->SetElement(0, ExtrapolatePatches_->isChecked());

    this->setModified();
}


void pqPV3FoamReaderPanel::InterpolateVolFieldsToggled()
{
    vtkSMIntVectorProperty::SafeDownCast
    (
        this->proxy()->GetProperty("UiInterpolateVolFields")
    )->SetElement(0, InterpolateVolFields_->isChecked());

    this->setModified();
}


// ************************************************************************* //
