#ifndef _FRAGTRACECONTROLPANEL_H_
#define _FRAGTRACECONTROLPANEL_H_

#include "v3d_compile_full.h"
#include "v3d_interface.h"
#include "INeuronAssembler.h"
#include "IPMain4NeuronAssembler.h"

#include "FragTracer_Define.h"
#include "ui_fragmentedTraceUI.h"
#include "FragTraceManager.h"
#include "FragmentEditor.h"

class FragTraceControlPanel : public QDialog, public IPMain4NeuronAssembler
{
	Q_OBJECT
	Q_INTERFACES(IPMain4NeuronAssembler)

	friend class FragmentedAutoTracePlugin;
	//friend class CViewer;

public:
	FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback);

	// ======= Saving path for results / intermediate results ======= //
	QString saveSWCFullName;
	QString adaSaveRoot;
	QString histMaskRoot;
	// ============================================================== //


	/* ======= Result and Scaling Functions ======= */ 
	NeuronTree tracedTree;
	map<string, NeuronTree> tracedTrees;
	map<int, string> scalingRatioMap;
	void scaleTracedTree();
	NeuronTree treeScaleBack(const NeuronTree& inputTree);
	/* ============================================ */

	map<string, float> paramsFromUI;

	vector<string> blankAreas; // will be abandoned

	/* ======= Terafly Communicating Methods ======= */
	virtual void getNAVersionNum();

	virtual void updateCViewerPortal();

	virtual bool markerMonitorStatus() { return this->uiPtr->groupBox_15->isChecked(); }
	virtual void sendSelectedMarkers2NA(const QList<ImageMarker>& selectedMarkerList, const QList<ImageMarker>& selectedLocalMarkerList);

	virtual void eraserSegProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[], const int mouseX, const int mouseY);
	/* ============================================= */


public slots:
    /* ================== User Interface Buttons =================== */
	// ------- Configuration ------- //
	void imgFmtChecked(bool checked);
	void nestedChecks(bool checked);
	void multiSomaTraceChecked(bool checked);
	void saveSegStepsResultChecked(bool checked);
	void saveSettingsClicked();
	void browseSavePathClicked();
	void blankAreaClicked();
	// ---------------------------- //

	// ------- Post Editing ------- //
	void eraseButtonClicked();
	// ---------------------------- //
    /* ======= END of [User Interface Configuration Buttons] ======= */


	// ********************************************************************** //
	void traceButtonClicked(); // ==> THIS IS WHERE THE TRACING PROCESS STARTS
	// ********************************************************************** //


	// ------- Receive traced tree emitted from FragTraceManager ------- //
	void catchTracedTree(NeuronTree tracedTree) { this->tracedTree = tracedTree; }
	// ----------------------------------------------------------------- //


private:
	V3DPluginCallback2* thisCallback;
	Ui::FragmentedTraceUI* uiPtr;
	FragTraceManager* traceManagerPtr;
	FragmentEditor* fragEditorPtr;
	INeuronAssembler* CViewerPortal;


	/* =============== Additional Widget =============== */
	QDoubleSpinBox* doubleSpinBox;
	QStandardItemModel* listViewBlankAreas;
	QStandardItemModel* somaListViewer;
	/* ================================================= */


	/* ======= Marker Detection ======= */
	int surType;
	QList<ImageMarker> updatedMarkerList;
	QList<ImageMarker> selectedMarkerList;
	QList<ImageMarker> selectedLocalMarkerList;
	map<int, ImageMarker> somaMap;
	map<int, ImageMarker> localSomaMap;
	map<int, string> somaDisplayNameMap;
	void updateMarkerMonitor();
	/* ================================ */


	/* =============== Parameter Collecting Functions =============== */
	void pa_imgEnhancement();
	void pa_maskGeneration();
	void pa_objFilter();
	void pa_objBasedMST();
	/* =========== END of [Parameter Collecting Functions] ========== */


	/* ======= Tracing Volume Preparation ======= */
	// Partial volume tracing is achieved by talking to tf::PluginInterface through V3d_PluginLoader with v3d_interface's virtual [getPartialVolumeCoords],
	// so that it can be directly accessed through [thisCalback] from [teraflyTracePrep].
	bool volumeAdjusted;
	int* volumeAdjustedCoords; // local coordinates of [displaying image boudaries], eg, 1 ~ 256, etc
	int* globalCoords;         // global coordinates of [displaying image boundaries] in whole brain scale, currently not used.
	int* displayingDims;

	void teraflyTracePrep(workMode mode); // Image preparation; NOTE: FragTraceManager is created here!
	/* ========================================== */


	void fillUpParamsForm(); // This is for future parameter learning project.

	void blankArea(); // will be abandoned in newer version


private slots:	
	void refreshSomaCoords();



// ~~~~~~~~~ DEPRECATED FUNCTIONS ~~~~~~~~~ //
signals:
	void switchOnSegPipe();
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
};


#endif