#include "deviceKeyModel.h"
#include "deviceNotificationServer.h"
#include "deviceProxyWrapper.h"

#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "parameterSet.h"

#include "message/device.proxy.h"

#include <precitec/userManagement.h>

#include <QCoreApplication>
#include <QFutureWatcher>
#include <QtConcurrentRun>

using namespace precitec::interface;
using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

namespace
{
QString commentForKey(const std::string &key)
{
    static const std::map<std::string, std::pair<const char*, const char *>> s_comments = {
        {std::string("IsParallelInspection"), QT_TRANSLATE_NOOP3("", "Parallel processing active", "Precitec.KeyValue.IsParallelInspection")},
        {std::string("DisableHWResults"), QT_TRANSLATE_NOOP3("", "Disable the generation of any hw related results", "Precitec.KeyValue.DisableHWResults")},
        {std::string("SM_UseGridRecognition"), QT_TRANSLATE_NOOP3("", "Use grid recognition to improve Scanmaster calibration", "Precitec.KeyValue.SM_UseGridRecognition")},
        {std::string("IsEnabled"), QT_TRANSLATE_NOOP3("", "Video recorder active.", "Precitec.KeyValue.IsEnabled")},
        {std::string("NioMode"), QT_TRANSLATE_NOOP3("", "Which images are recorded. 0 - all images are recorded.  1 - Only images with an NOK measured are recorded. 2 - Only seam sequences with one or more NOK measured are recorded.", "Precitec.KeyValue.NioMode")},
        {std::string("SkipIfFull"), QT_TRANSLATE_NOOP3("", "Skipping frames when queue is full.", "Precitec.KeyValue.FailOnFull")},
        {std::string("ProductTypeStart"), QT_TRANSLATE_NOOP3("", "Product number at which recording starts.", "Precitec.KeyValue.ProductTypeStart")},
        {std::string("ProductTypeEnd"), QT_TRANSLATE_NOOP3("", "Product number at which recording ends.", "Precitec.KeyValue.ProductTypeEnd")},
        {std::string("SeamSeriesStart"), QT_TRANSLATE_NOOP3("", "Seam series at which recording starts.", "Precitec.KeyValue.SeamSeriesStart")},
        {std::string("SeamSeriesEnd"), QT_TRANSLATE_NOOP3("", "Seam series at which recording ends.", "Precitec.KeyValue.SeamSeriesEnd")},
        {std::string("SeamStart"), QT_TRANSLATE_NOOP3("", "Seam at which recording starts.", "Precitec.KeyValue.SeamStart")},
        {std::string("SeamEnd"), QT_TRANSLATE_NOOP3("", "Seam at which recording ends.", "Precitec.KeyValue.SeamEnd")},
        {std::string("NbProductsToKeep"), QT_TRANSLATE_NOOP3("", "Number of recorded products to be hold on disk. Older products are automatically deleted.", "Precitec.KeyValue.NbProductsToKeep")},
        {std::string("NbLiveModeToKeep"), QT_TRANSLATE_NOOP3("", "Number of recorded live mode products to be hold on disk. Older live mode products are automatically deleted.", "Precitec.KeyValue.NbLiveModeToKeep")},
        {std::string("NbMaxImages"), QT_TRANSLATE_NOOP3("", "Maximum number of images to be recorded in a live mode sequence.", "Precitec.KeyValue.NbMaxImages")},
        {std::string("MaxDiskUsage"), QT_TRANSLATE_NOOP3("", "Maximum usage of disk space.", "Precitec.KeyValue.MaxDiskUsage")},
        {std::string("beta0"), QT_TRANSLATE_NOOP3("", "Info: Magnification", "Precitec.KeyValue.beta0")},
        {std::string("XLeftEdge0"), QT_TRANSLATE_NOOP3("", "Info: x-coordinate of left edge of ROI for line laser 1", "Precitec.KeyValue.XLeftEdge0")},
        {std::string("XRightEdge0"), QT_TRANSLATE_NOOP3("", "Info: x-coordinate of right edge of ROI for line laser 1", "Precitec.KeyValue.XRightEdge0")},
        {std::string("YTopEdge0"), QT_TRANSLATE_NOOP3("", "Info: y-coordinate of upper edge of ROI for line laser 1", "Precitec.KeyValue.YTopEdge0")},
        {std::string("YBottomEdge0"), QT_TRANSLATE_NOOP3("", "Info: y-coordinate of lower edge of ROI for line laser 1", "Precitec.KeyValue.YBottomEdge0")},
        {std::string("XLeftEdgeTCP"), QT_TRANSLATE_NOOP3("", "Info: x-coordinate of left edge of ROI for line laser 3", "Precitec.KeyValue.XLeftEdgeTCP")},
        {std::string("XRightEdgeTCP"), QT_TRANSLATE_NOOP3("", "Info: x-coordinate of right edge of ROI for line laser 3", "Precitec.KeyValue.XRightEdgeTCP")},
        {std::string("YTopEdgeTCP"), QT_TRANSLATE_NOOP3("", "Info: y-coordinate of upper edge of ROI for line laser 3", "Precitec.KeyValue.YTopEdgeTCP")},
        {std::string("YBottomEdgeTCP"), QT_TRANSLATE_NOOP3("", "Info: y-coordinate of lower edge of ROI laser line in 3", "Precitec.KeyValue.YBottomEdgeTCP")},
        {std::string("XLeftEdge2"), QT_TRANSLATE_NOOP3("", "Info: x-coordinate of left edge of ROI for line laser 2", "Precitec.KeyValue.XLeftEdge2")},
        {std::string("XRightEdge2"), QT_TRANSLATE_NOOP3("", "Info: x-coordinate of right edge of ROI for line laser 2", "Precitec.KeyValue.XRightEdge2")},
        {std::string("YTopEdge2"), QT_TRANSLATE_NOOP3("", "Info: y-coordinate of upper edge of ROI for line laser 2", "Precitec.KeyValue.YTopEdge2")},
        {std::string("YBottomEdge2"), QT_TRANSLATE_NOOP3("", "Info: y-coordinate of lower edge of ROI for line laser 2", "Precitec.KeyValue.YBottomEdge2")},
        {std::string("g0"), QT_TRANSLATE_NOOP3("", "Input: Focussed object distance g0 [mm]", "Precitec.KeyValue.g0")},
        {std::string("Cf"), QT_TRANSLATE_NOOP3("", "Info: Principal distance [mm]", "Precitec.KeyValue.Cf")},
        {std::string("alphaX"), QT_TRANSLATE_NOOP3("", "Info: Camera calibration matrix K11 [Pixel]", "Precitec.KeyValue.alphaX")},
        {std::string("alphaY"), QT_TRANSLATE_NOOP3("", "Info: Camera calibration matrix K22 [Pixel]", "Precitec.KeyValue.alphaY")},
        {std::string("skew"), QT_TRANSLATE_NOOP3("", "Info: Camera calibration matrix K12 [Pixel]", "Precitec.KeyValue.skew")},
        {std::string("xcc"), QT_TRANSLATE_NOOP3("", "Input: Camera principal point X coordinate [Pixel]", "Precitec.KeyValue.xcc")},
        {std::string("ycc"), QT_TRANSLATE_NOOP3("", "Input: Camera principal point Y coordinate [Pixel]", "Precitec.KeyValue.ycc")},
        {std::string("DpixX"), QT_TRANSLATE_NOOP3("", "Input: Camera pixel diameter horizontal [mm]", "Precitec.KeyValue.DpixX")},
        {std::string("DpixY"), QT_TRANSLATE_NOOP3("", "Input: Camera pixel diameter vertical [mm]", "Precitec.KeyValue.DpixY")},
        {std::string("p1"), QT_TRANSLATE_NOOP3("", "Info: Triangulation plane element 1 [mm]", "Precitec.KeyValue.p1")},
        {std::string("p2"), QT_TRANSLATE_NOOP3("", "Info: Triangulation plane element 2 [mm]", "Precitec.KeyValue.p2")},
        {std::string("p3"), QT_TRANSLATE_NOOP3("", "Info: Triangulation plane element 3 [mm]", "Precitec.KeyValue.p3")},
        {std::string("p4"), QT_TRANSLATE_NOOP3("", "Info: Triangulation plane element 4 [mm]", "Precitec.KeyValue.p4")},
        {std::string("ex"), QT_TRANSLATE_NOOP3("", "Info: Tracking axis unit vector x", "Precitec.KeyValue.ex")},
        {std::string("ey"), QT_TRANSLATE_NOOP3("", "Info: Tracking axis unit vector y", "Precitec.KeyValue.ey")},
        {std::string("ez"), QT_TRANSLATE_NOOP3("", "Info: Tracking axis unit vector z", "Precitec.KeyValue.ez")},
        {std::string("mmPerIncrement"), QT_TRANSLATE_NOOP3("", "Input: Tracking axis increment length [mm]", "Precitec.KeyValue.mmPerIncrement")},
        {std::string("beta0min"), QT_TRANSLATE_NOOP3("", "Input: Calibration of magnification, minimal acceptable value", "Precitec.KeyValue.beta0min")},
        {std::string("beta0max"), QT_TRANSLATE_NOOP3("", "Input: Calibration of magnification, maximal acceptable value", "Precitec.KeyValue.beta0max")},
        {std::string("cal_dg_left"), QT_TRANSLATE_NOOP3("", "Input: Calibration part left image side deviation from g0 [mm]", "Precitec.KeyValue.cal_dg_left")},
        {std::string("cal_dg_right"), QT_TRANSLATE_NOOP3("", "Input: Calibration part right image side deviation from g0 [mm]", "Precitec.KeyValue.cal_dg_right")},
        {std::string("sensorNPixX"), QT_TRANSLATE_NOOP3("", "Input: Number of pixel in camera row (x) [Pixel]", "Precitec.KeyValue.sensorNPixX")},
        {std::string("sensorNPixY"), QT_TRANSLATE_NOOP3("", "Input: Number of pixel in camera column (y) [Pixel]", "Precitec.KeyValue.sensorNPixY")},
        {std::string("triangulationCalEvalAreaDiameter"), QT_TRANSLATE_NOOP3("", "Input: Diameter of axis movement during triangulation calibration [mm]", "Precitec.KeyValue.triangulationCalEvalAreaDiameter")},
        {std::string("xtcp"), QT_TRANSLATE_NOOP3("", "Info: TCP image coordinate horizontal (x) [Pixel]", "Precitec.KeyValue.xtcp")},
        {std::string("ytcp"), QT_TRANSLATE_NOOP3("", "Info: TCP image coordinate vertical (y) [Pixel]", "Precitec.KeyValue.ytcp")},
        {std::string("ScheimpflugAngleDegAlpha1"), QT_TRANSLATE_NOOP3("", "Input: Scheimpflug tilt angle [deg]", "Precitec.KeyValue.ScheimpflugAngleDegAlpha1")},
        {std::string("ScheimpflugCameraModel"), QT_TRANSLATE_NOOP3("", "0: AA100 1: AA150 2: AA200 3: F64 Calibration Pattern size is chosen accordingly", "Precitec.KeyValue.ScheimpflugCameraModel")},
        {std::string("ScheimpflugCalib_Threshold"), QT_TRANSLATE_NOOP3("", "Threshold for the chessboard recognition procedure. -1: Automatic estimation", "Precitec.KeyValue.ScheimpflugCalib_Threshold")},
        {std::string("MarginToFrameRatio"), QT_TRANSLATE_NOOP3("", "Input: Relative margin size of calibration images", "Precitec.KeyValue.MarginToFrameRatio")},
        {std::string("ExposureTime"), QT_TRANSLATE_NOOP3("", "Exposure time. ( 0..120 )", "Precitec.KeyValue.ExposureTime")},
        {std::string("Voltages.BlackLevelOffset"), QT_TRANSLATE_NOOP3("", "Blacklevel offset: If there is no light falling on the sensor ,the average image brightness should be between 0 and 3. (0..4095, Qualas: 0..16383)", "Precitec.KeyValue.Voltages.BlackLevelOffset")},
        {std::string("BlackLevelOffset"), QT_TRANSLATE_NOOP3("", "Info: Brightness adjustment value of camera chip. Is used for LED calibartion", "Precitec.KeyValue.BlackLevelOffset")},
        {std::string("Trigger.Source"), QT_TRANSLATE_NOOP3("", "Determines trigger source: o free running; 1 interface trigger(default); 2 IO trigger; ", "Precitec.KeyValue.Trigger.Source")},
        {std::string("Trigger.Interleave"), QT_TRANSLATE_NOOP3("", "1: Enable simultaneous readout. Combination with Skim and LevelControlled is not available! ", "Precitec.KeyValue.Trigger.Interleave")},
        {std::string("StoreDefaults"), QT_TRANSLATE_NOOP3("", "Write camera parameters to persitent  internal memory.", "Precitec.KeyValue.StoreDefaults")},
        {std::string("LinLog.Mode"), QT_TRANSLATE_NOOP3("", "Activate (1,2,3,4) or deactivate (0) camera LinLog mode.", "Precitec.KeyValue.LinLog.Mode")},
        {std::string("LinLog.Value1"), QT_TRANSLATE_NOOP3("", "LinLog value 1 (0..30). Must NOT be set below LinLog value 2.", "Precitec.KeyValue.LinLog.Value1")},
        {std::string("LinLog.Value2"), QT_TRANSLATE_NOOP3("", "LinLog value 2 (0..30). Must NOT exceed LinLog value 1.", "Precitec.KeyValue.LinLog.Value2")},
        {std::string("LinLog.Time1"), QT_TRANSLATE_NOOP3("", "LinLog time T1  (1..1000), per mill of exposure time. Must NOT exceed LinLog time T2.", "Precitec.KeyValue.LinLog.Time1")},
        {std::string("LinLog.Time2"), QT_TRANSLATE_NOOP3("", "LinLog time T2 (1..1000), per mill of exposure time. Must NOT be set below T1.", "Precitec.KeyValue.LinLog.Time2")},
        {std::string("Window.X"), QT_TRANSLATE_NOOP3("", "Horizontal offset (x-direction) of the camera ROI of interest w.r.t. the internal image sensor. Margin to left border in pixel (0..1012).", "Precitec.KeyValue.Window.X")},
        {std::string("Window.Y"), QT_TRANSLATE_NOOP3("", "Vertical offset (y-direction) of the camera ROI of interest w.r.t. the internal image sensor. Margin to top border in pixel (0..1023).", "Precitec.KeyValue.Window.Y")},
        {std::string("Window.W"), QT_TRANSLATE_NOOP3("", "Width of camera ROI w.r.t. the internal sensor. Pixel (12..1024).", "Precitec.KeyValue.Window.W")},
        {std::string("Window.H"), QT_TRANSLATE_NOOP3("", "Height of camera ROI w.r.t. the internal sensor. Pixel (1..1024).", "Precitec.KeyValue.Window.H")},
        {std::string("Device1_Process0_Trigger_TriggerMode"), QT_TRANSLATE_NOOP3("", "Triggermodus der Kamera setzen, sw controlled:2, grabber controlled:1", "Precitec.KeyValue.Device1_Process0_Trigger_TriggerMode")},
        {std::string("Device1_Process0_Trigger_ExsyncFramesPerSec"), QT_TRANSLATE_NOOP3("", "Number of images (1..1000) to be shot per second  (grabber controlled mode)", "Precitec.KeyValue.Device1_Process0_Trigger_ExsyncFramesPerSec")},
        {std::string("Device1_Process0_Trigger_ExsyncExposure"), QT_TRANSLATE_NOOP3("", "Length of the exposure puls", "Precitec.KeyValue.Device1_Process0_Trigger_ExsyncExposure")},
        {std::string("Device1_Process0_Trigger_ExsyncDelay"), QT_TRANSLATE_NOOP3("", "Exsync signal delay", "Precitec.KeyValue.Device1_Process0_Trigger_ExsyncDelay")},
        {std::string("Device1_Process0_Trigger_ExsyncPolarity"), QT_TRANSLATE_NOOP3("", "Polarity of the ExSync Signal -0 high 1: low", "Precitec.KeyValue.Device1_Process0_Trigger_ExsyncPolarity")},
        {std::string("Device1_Process0_Trigger_FlashEnable"), QT_TRANSLATE_NOOP3("", "LED illumination - on:1 off:0", "Precitec.KeyValue.Device1_Process0_Trigger_FlashEnable")},
        {std::string("PWM.PWM1"), QT_TRANSLATE_NOOP3("", "Line laser intensity at camera output 1 (0…255)", "Precitec.KeyValue.PWM.PWM1")},
        {std::string("PWM.PWM2"), QT_TRANSLATE_NOOP3("", "Line laser intensity at camera output 2 (0…255)", "Precitec.KeyValue.PWM.PWM2")},
        {std::string("PWM.PWMBel"), QT_TRANSLATE_NOOP3("", "Field illumination intensity at camera output 3 (0…255)", "Precitec.KeyValue.PWM.PWMBel")},
        {std::string("PWM.Enable"), QT_TRANSLATE_NOOP3("", "Line laser on via camera ouput", "Precitec.KeyValue.PWM.Enable")},
        {std::string("Skim"), QT_TRANSLATE_NOOP3("", "Amplification in low light situations (0: disabled )", "Precitec.KeyValue.Skim")},
        {std::string("Correction.Mode"), QT_TRANSLATE_NOOP3("", "Image Correction: 0 - no correction, 1 - offset, 2 - offset u. hot pixel, 3 - hot pixel, 4 - offset and gain, 5 - offset, gain and hot pixel", "Precitec.KeyValue.Correction.Mode")},
        {std::string("Trigger.LevelControlled"), QT_TRANSLATE_NOOP3("", "Exposure Time Control: 0 software controlled, 1 reset pulswidth controlled", "Precitec.KeyValue.Trigger.LevelControlled")},
        {std::string("Device1_Process0_Debug_Output"), QT_TRANSLATE_NOOP3("", "Additional Debug Output. 0 - less output, 1 - more output", "Precitec.KeyValue.Device1_Process0_Debug_Output")},
        {std::string("PWM.FiberLossTimeout"), QT_TRANSLATE_NOOP3("", "Time in ms, before PWM.Enable will be set down in case of  fiberloss", "Precitec.KeyValue.PWM.FiberLossTimeout")},
        {std::string("CameraReOpen"), QT_TRANSLATE_NOOP3("", "Re-Initialising the camera port  with 1 ", "Precitec.KeyValue.CameraReOpen")},
        {std::string("FlushPort"), QT_TRANSLATE_NOOP3("", "Clear the camera communication channel", "Precitec.KeyValue.FlushPort")},
        {std::string("ScanTrackerFrequency.eFreq30"), QT_TRANSLATE_NOOP3("", "30Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq30")},
        {std::string("ScanTrackerFrequency.eFreq40"), QT_TRANSLATE_NOOP3("", "40Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq40")},
        {std::string("ScanTrackerFrequency.eFreq50"), QT_TRANSLATE_NOOP3("", "50Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq50")},
        {std::string("ScanTrackerFrequency.eFreq100"), QT_TRANSLATE_NOOP3("", "100Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq100")},
        {std::string("ScanTrackerFrequency.eFreq150"), QT_TRANSLATE_NOOP3("", "150Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq150")},
        {std::string("ScanTrackerFrequency.eFreq200"), QT_TRANSLATE_NOOP3("", "200Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq200")},
        {std::string("ScanTrackerFrequency.eFreq250"), QT_TRANSLATE_NOOP3("", "250Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq250")},
        {std::string("ScanTrackerFrequency.eFreq300"), QT_TRANSLATE_NOOP3("", "300Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq300")},
        {std::string("ScanTrackerFrequency.eFreq350"), QT_TRANSLATE_NOOP3("", "350Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq350")},
        {std::string("ScanTrackerFrequency.eFreq400"), QT_TRANSLATE_NOOP3("", "400Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq400")},
        {std::string("ScanTrackerFrequency.eFreq450"), QT_TRANSLATE_NOOP3("", "450Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq450")},
        {std::string("ScanTrackerFrequency.eFreq500"), QT_TRANSLATE_NOOP3("", "500Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq500")},
        {std::string("ScanTrackerFrequency.eFreq550"), QT_TRANSLATE_NOOP3("", "550Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq550")},
        {std::string("ScanTrackerFrequency.eFreq600"), QT_TRANSLATE_NOOP3("", "600Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq600")},
        {std::string("ScanTrackerFrequency.eFreq650"), QT_TRANSLATE_NOOP3("", "650Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq650")},
        {std::string("ScanTrackerFrequency.eFreq700"), QT_TRANSLATE_NOOP3("", "700Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq700")},
        {std::string("ScanTrackerFrequency.eFreq750"), QT_TRANSLATE_NOOP3("", "750Hz", "Precitec.KeyValue.ScanTrackerFrequency.eFreq750")},
        {std::string("TypeOfSensor.eKoaxHead"), QT_TRANSLATE_NOOP3("", "Coax head", "Precitec.KeyValue.TypeOfSensor.eKoaxHead")},
        {std::string("TypeOfSensor.eScheimpflugHead"), QT_TRANSLATE_NOOP3("", "Scheimpflug head", "Precitec.KeyValue.TypeOfSensor.eScheimpflugHead")},
        {std::string("TypeOfSensor.eLedHead"), QT_TRANSLATE_NOOP3("", "LED head", "Precitec.KeyValue.TypeOfSensor.eLedHead")},
        {std::string("LineLaser1OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off line laser 1", "Precitec.KeyValue.LineLaser1OnOff")},
        {std::string("LineLaser1Intensity"), QT_TRANSLATE_NOOP3("", "Line laser 1 intensity (0% - 100%)", "Precitec.KeyValue.LineLaser1Intensity")},
        {std::string("LineLaser2OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off line laser 2", "Precitec.KeyValue.LineLaser2OnOff")},
        {std::string("LineLaser2Intensity"), QT_TRANSLATE_NOOP3("", "Line laser 2 intensity (0% - 100%)", "Precitec.KeyValue.LineLaser2Intensity")},
        {std::string("FieldLight1OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off field illumination 1", "Precitec.KeyValue.FieldLight1OnOff")},
        {std::string("FieldLight1Intensity"), QT_TRANSLATE_NOOP3("", "Field illumination 1 intensity (0% - 100%)", "Precitec.KeyValue.FieldLight1Intensity")},
        {std::string("X_Axis_AbsolutePosition"), QT_TRANSLATE_NOOP3("", "Axis X: Drive to new absolute position", "Precitec.KeyValue.X_Axis_AbsolutePosition")},
        {std::string("X_Axis_SoftLimitsOnOff"), QT_TRANSLATE_NOOP3("", "Axis X: Switch on/off software limits", "Precitec.KeyValue.X_Axis_SoftLimitsOnOff")},
        {std::string("X_Axis_SetUpperLimit"), QT_TRANSLATE_NOOP3("", "Axis X: Position of upper software limit", "Precitec.KeyValue.X_Axis_SetUpperLimit")},
        {std::string("X_Axis_SetLowerLimit"), QT_TRANSLATE_NOOP3("", "Axis X: Position of lower software limit", "Precitec.KeyValue.X_Axis_SetLowerLimit")},
        {std::string("X_Axis_Velocity"), QT_TRANSLATE_NOOP3("", "Axis X: Velocity of axis move (1% - 100%)", "Precitec.KeyValue.X_Axis_Velocity")},
        {std::string("X_Axis_Acceleration"), QT_TRANSLATE_NOOP3("", "Axis X: Acceleration of axis move (1% - 100%)", "Precitec.KeyValue.X_Axis_Acceleration")},
        {std::string("X_Axis_HomingDirectionPositive"), QT_TRANSLATE_NOOP3("", "Axis X: Switch on/off homing to positive hardware limitswitch", "Precitec.KeyValue.X_Axis_HomingDirectionPositive")},
        {std::string("Y_Axis_AbsolutePosition"), QT_TRANSLATE_NOOP3("", "Axis Y: Drive to new absolute position", "Precitec.KeyValue.Y_Axis_AbsolutePosition")},
        {std::string("Y_Axis_SoftLimitsOnOff"), QT_TRANSLATE_NOOP3("", "Axis Y: Switch on/off software limits", "Precitec.KeyValue.Y_Axis_SoftLimitsOnOff")},
        {std::string("Y_Axis_SetUpperLimit"), QT_TRANSLATE_NOOP3("", "Axis Y: Position of upper software limit", "Precitec.KeyValue.Y_Axis_SetUpperLimit")},
        {std::string("Y_Axis_SetLowerLimit"), QT_TRANSLATE_NOOP3("", "Axis Y: Position of lower software limit", "Precitec.KeyValue.Y_Axis_SetLowerLimit")},
        {std::string("Y_Axis_Velocity"), QT_TRANSLATE_NOOP3("", "Axis Y: Velocity of axis move (1% - 100%)", "Precitec.KeyValue.Y_Axis_Velocity")},
        {std::string("Y_Axis_Acceleration"), QT_TRANSLATE_NOOP3("", "Axis Y: Acceleration of axis move (1% - 100%)", "Precitec.KeyValue.Y_Axis_Acceleration")},
        {std::string("Y_Axis_HomingDirectionPositive"), QT_TRANSLATE_NOOP3("", "Axis Y: Switch on/off homing to positive hardware limitswitch", "Precitec.KeyValue.Y_Axis_HomingDirectionPositive")},
        {std::string("Z_Axis_AbsolutePosition"), QT_TRANSLATE_NOOP3("", "Axis Z: Drive to new absolute position", "Precitec.KeyValue.Z_Axis_AbsolutePosition")},
        {std::string("Z_Axis_SoftLimitsOnOff"), QT_TRANSLATE_NOOP3("", "Axis Z: Switch on/off software limits", "Precitec.KeyValue.Z_Axis_SoftLimitsOnOff")},
        {std::string("Z_Axis_SetUpperLimit"), QT_TRANSLATE_NOOP3("", "Axis Z: Position of upper software limit", "Precitec.KeyValue.Z_Axis_SetUpperLimit")},
        {std::string("Z_Axis_SetLowerLimit"), QT_TRANSLATE_NOOP3("", "Axis Z: Position of lower software limit", "Precitec.KeyValue.Z_Axis_SetLowerLimit")},
        {std::string("Z_Axis_Velocity"), QT_TRANSLATE_NOOP3("", "Axis Z: Velocity of axis move (1% - 100%)", "Precitec.KeyValue.Z_Axis_Velocity")},
        {std::string("Z_Axis_Acceleration"), QT_TRANSLATE_NOOP3("", "Axis Z: Acceleration of axis move (1% - 100%)", "Precitec.KeyValue.Z_Axis_Acceleration")},
        {std::string("Z_Axis_HomingDirectionPositive"), QT_TRANSLATE_NOOP3("", "Axis Z: Switch on/off homing to positive hardware limitswitch", "Precitec.KeyValue.Z_Axis_HomingDirectionPositive")},
        {std::string("TrackerDriverOnOff"), QT_TRANSLATE_NOOP3("", "ScanTracker: Switch on/off scan function", "Precitec.KeyValue.TrackerDriverOnOff")},
        {std::string("ScanTrackerFrequency"), QT_TRANSLATE_NOOP3("", "ScanTracker: Frequency for scan function", "Precitec.KeyValue.ScanTrackerFrequency")},
        {std::string("ScanWidthOutOfGapWidth"), QT_TRANSLATE_NOOP3("", "ScanTracker: Dynamic scan-width determined by graph", "Precitec.KeyValue.ScanWidthOutOfGapWidth")},
        {std::string("ScanWidthFixed"), QT_TRANSLATE_NOOP3("", "ScanTracker: Fixed scan-width", "Precitec.KeyValue.ScanWidthFixed")},
        {std::string("GapWidthToScanWidthOffset"), QT_TRANSLATE_NOOP3("", "ScanTracker: Offset for linear function gapwidth to scanwidth", "Precitec.KeyValue.GapWidthToScanWidthOffset")},
        {std::string("GapWidthToScanWidthGradient"), QT_TRANSLATE_NOOP3("", "ScanTracker: Gradient for linear function gapwidth to scanwidth", "Precitec.KeyValue.GapWidthToScanWidthGradient")},
        {std::string("ScanPosOutOfGapPos"), QT_TRANSLATE_NOOP3("", "ScanTracker: Dynamic scan-position determined by graph", "Precitec.KeyValue.ScanPosOutOfGapPos")},
        {std::string("ScanPosFixed"), QT_TRANSLATE_NOOP3("", "ScanTracker: Fixed scan-position", "Precitec.KeyValue.ScanPosFixed")},
        {std::string("ScanTrackerAskStatus"), QT_TRANSLATE_NOOP3("", "ScanTracker: Query ScanTracker status via serial interface", "Precitec.KeyValue.ScanTrackerAskStatus")},
        {std::string("ScanTrackerAskRevisions"), QT_TRANSLATE_NOOP3("", "ScanTracker: Query ScanTracker revisions via serial interface", "Precitec.KeyValue.ScanTrackerAskRevisions")},
        {std::string("ScanTrackerAskSerialNumbers"), QT_TRANSLATE_NOOP3("", "ScanTracker: Query ScanTracker serial numbers via serial interface", "Precitec.KeyValue.ScanTrackerAskSerialNumbers")},
        {std::string("TrackerMaxAmplitude"), QT_TRANSLATE_NOOP3("", "ScanTracker: the ScanTracker is only allowed to deflect this maximum amplitude out of zero-point (positive resp. negative)", "Precitec.KeyValue.TrackerMaxAmplitude")},
        {std::string("FocalLength"), QT_TRANSLATE_NOOP3("", "Focal length", "Precitec.KeyValue.FocalLength")},
        {std::string("ClearEncoderCounter1"), QT_TRANSLATE_NOOP3("", "Encoder counter 1 is reset", "Precitec.KeyValue.ClearEncoderCounter1")},
        {std::string("ClearEncoderCounter2"), QT_TRANSLATE_NOOP3("", "Encoder counter 2 is reset", "Precitec.KeyValue.ClearEncoderCounter2")},
        {std::string("LEDFlashDelay"), QT_TRANSLATE_NOOP3("", "LED Flash Delay", "Precitec.KeyValue.Device1_Process0_Trigger_FlashDelay")},
        {std::string("LEDPanel1OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off panel 1 of LED illumination", "Precitec.KeyValue.LEDPanel1OnOff")},
        {std::string("LEDPanel1Intensity"), QT_TRANSLATE_NOOP3("", "Intensity of panel 1 of LED illumination (0% - 100%)", "Precitec.KeyValue.LEDPanel1Intensity")},
        {std::string("LEDPanel1PulseWidth"), QT_TRANSLATE_NOOP3("", "Switched on pulse width of LED panel 1 (50us - 400us)", "Precitec.KeyValue.LEDPanel1PulseWidth")},
        {std::string("LEDPanel2OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off panel 2 of LED illumination", "Precitec.KeyValue.LEDPanel2OnOff")},
        {std::string("LEDPanel2Intensity"), QT_TRANSLATE_NOOP3("", "Intensity of panel 2 of LED illumination (0% - 100%)", "Precitec.KeyValue.LEDPanel2Intensity")},
        {std::string("LEDPanel2PulseWidth"), QT_TRANSLATE_NOOP3("", "Switched on pulse width of LED panel 2 (50us - 400us)", "Precitec.KeyValue.LEDPanel2PulseWidth")},
        {std::string("LEDPanel3OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off panel 3 of LED illumination", "Precitec.KeyValue.LEDPanel3OnOff")},
        {std::string("LEDPanel3Intensity"), QT_TRANSLATE_NOOP3("", "Intensity of panel 3 of LED illumination (0% - 100%)", "Precitec.KeyValue.LEDPanel3Intensity")},
        {std::string("LEDPanel3PulseWidth"), QT_TRANSLATE_NOOP3("", "Switched on pulse width of LED panel 3 (50us - 400us)", "Precitec.KeyValue.LEDPanel3PulseWidth")},
        {std::string("LEDPanel4OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off panel 4 of LED illumination", "Precitec.KeyValue.LEDPanel4OnOff")},
        {std::string("LEDPanel4Intensity"), QT_TRANSLATE_NOOP3("", "Intensity of panel 4 of LED illumination (0% - 100%)", "Precitec.KeyValue.LEDPanel4Intensity")},
        {std::string("LEDPanel4PulseWidth"), QT_TRANSLATE_NOOP3("", "Switched on pulse width of LED panel 4 (50us - 400us)", "Precitec.KeyValue.LEDPanel4PulseWidth")},
        {std::string("LEDPanel5OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off panel 5 of LED illumination", "Precitec.KeyValue.LEDPanel5OnOff")},
        {std::string("LEDPanel5Intensity"), QT_TRANSLATE_NOOP3("", "Intensity of panel 5 of LED illumination (0% - 100%)", "Precitec.KeyValue.LEDPanel5Intensity")},
        {std::string("LEDPanel5PulseWidth"), QT_TRANSLATE_NOOP3("", "Switched on pulse width of LED panel 5 (50us - 400us)", "Precitec.KeyValue.LEDPanel5PulseWidth")},
        {std::string("LEDPanel6OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off panel 6 of LED illumination", "Precitec.KeyValue.LEDPanel6OnOff")},
        {std::string("LEDPanel6Intensity"), QT_TRANSLATE_NOOP3("", "Intensity of panel 6 of LED illumination (0% - 100%)", "Precitec.KeyValue.LEDPanel6Intensity")},
        {std::string("LEDPanel6PulseWidth"), QT_TRANSLATE_NOOP3("", "Switched on pulse width of LED panel 6 (50us - 400us)", "Precitec.KeyValue.LEDPanel6PulseWidth")},
        {std::string("LEDPanel7OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off panel 7 of LED illumination", "Precitec.KeyValue.LEDPanel7OnOff")},
        {std::string("LEDPanel7Intensity"), QT_TRANSLATE_NOOP3("", "Intensity of panel 7 of LED illumination (0% - 100%)", "Precitec.KeyValue.LEDPanel7Intensity")},
        {std::string("LEDPanel7PulseWidth"), QT_TRANSLATE_NOOP3("", "Switched on pulse width of LED panel 7 (50us - 400us)", "Precitec.KeyValue.LEDPanel7PulseWidth")},
        {std::string("LEDPanel8OnOff"), QT_TRANSLATE_NOOP3("", "Switch on/off panel 8 of LED illumination", "Precitec.KeyValue.LEDPanel8OnOff")},
        {std::string("LEDPanel8Intensity"), QT_TRANSLATE_NOOP3("", "Intensity of panel 8 of LED illumination (0% - 100%)", "Precitec.KeyValue.LEDPanel8Intensity")},
        {std::string("LEDPanel8PulseWidth"), QT_TRANSLATE_NOOP3("", "Switched on pulse width of LED panel 8 (50us - 400us)", "Precitec.KeyValue.LEDPanel8PulseWidth")},
        {std::string("LED_SaveIntensityPersistent"), QT_TRANSLATE_NOOP3("", "save current intensity adjustments of LED illumination permanent", "Precitec.KeyValue.LED_SaveIntensityPersistent")},
        {std::string("ledCalibrationChannel"), QT_TRANSLATE_NOOP3("", "which led channel is being calibrated at the moment", "Precitec.KeyValue.ledCalibrationChannel")},
        {std::string("LEDSendData"), QT_TRANSLATE_NOOP3("", "Send data to LED controller", "Precitec.KeyValue.LEDSendData")},
        {std::string("LC_Send_Data"), QT_TRANSLATE_NOOP3("", "Send data to LaserControl device", "Precitec.KeyValue.LC_Send_Data")},
        {std::string("LC_Parameter_No1"), QT_TRANSLATE_NOOP3("", "Mode of LaserControl (1,2,3)", "Precitec.KeyValue.LC_Parameter_No1")},
        {std::string("LC_Parameter_No2"), QT_TRANSLATE_NOOP3("", "Offset of wave to ScanTracker motion", "Precitec.KeyValue.LC_Parameter_No2")},
        {std::string("LC_Parameter_No3"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No3")},
        {std::string("LC_Parameter_No4"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No4")},
        {std::string("LC_Parameter_No5"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No5")},
        {std::string("LC_Parameter_No6"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No6")},
        {std::string("LC_Parameter_No7"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No7")},
        {std::string("LC_Parameter_No8"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No8")},
        {std::string("LC_Parameter_No9"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No9")},
        {std::string("LC_Parameter_No10"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No10")},
        {std::string("LC_Parameter_No11"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No11")},
        {std::string("LC_Parameter_No12"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No12")},
        {std::string("LC_Parameter_No13"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No13")},
        {std::string("LC_Parameter_No14"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No14")},
        {std::string("LC_Parameter_No15"), QT_TRANSLATE_NOOP3("", "", "Precitec.KeyValue.LC_Parameter_No15")},
        {std::string("betaZ"), QT_TRANSLATE_NOOP3("", "Info: Vertical magnification (y coord delta) &lt;-&gt; mm delta on line laser 1", "Precitec.KeyValue.betaZ")},
        {std::string("betaZ_2"), QT_TRANSLATE_NOOP3("", "Info: Vertical magnification (y coord delta) &lt;-&gt; mm delta on line laser 2", "Precitec.KeyValue.betaZ_2")},
        {std::string("betaZ_TCP"), QT_TRANSLATE_NOOP3("", "Info: Vertical magnification (y coord delta) &lt;-&gt; mm delta on line laser 3", "Precitec.KeyValue.betaZ_TCP")},
        {std::string("HighPlaneOnImageTop"), QT_TRANSLATE_NOOP3("", "Info: direction of line laser 1.\n1: Laser line comes from the top of the image (higher Z is at the top of the image )\n0: Laser line comes from the bottom of the image (higher Z is at the bottom of the image )", "Precitec.KeyValue.HighPlaneOnImageTop")},
        {std::string("HighPlaneOnImageTop_2"), QT_TRANSLATE_NOOP3("", "Info: direction of line laser 2.\n1: Laser line comes from the top of the image (higher Z is at the top of the image )\n0: Laser line comes from the bottom of the image (higher Z is at the bottom of the image )", "Precitec.KeyValue.HighPlaneOnImageTop_2")},
        {std::string("HighPlaneOnImageTop_TCP"), QT_TRANSLATE_NOOP3("", "Info: direction of line laser 3.\n1: Laser line comes from the top of the image (higher Z is at the top of the image )\n0: Laser line comes from the bottom of the image (higher Z is at the bottom of the image )", "Precitec.KeyValue.HighPlaneOnImageTop_TCP")},
        {std::string("triangulationAngle"), QT_TRANSLATE_NOOP3("", "Info: Triangulation angle of line laser 1 [degrees]. A positive value  means that higher Z is at the top of the image. This value is automatically recomputed  when beta0, betaZ or HighPlaneOnImageTop are manually edited.", "Precitec.KeyValue.triangulationAngle")},
        {std::string("triangulationAngle_2"), QT_TRANSLATE_NOOP3("", "Info: Triangulation angle of line laser 2 line [degrees]. A positive value  means that higher Z is at the top of the image. This value is automatically recomputed  when beta0, betaZ_2 or HighPlaneOnImageTop_2 are manually edited.", "Precitec.KeyValue.triangulationAngle_2")},
        {std::string("triangulationAngle_TCP"), QT_TRANSLATE_NOOP3("", "Info: Triangulation angle of line laser 3 [degrees]. A positive value  means that higher Z is at the top of the image. This value is automatically recomputed  when beta0, betaZ_TCP or HighPlaneOnImageTop_TCP are manually edited.", "Precitec.KeyValue.triangulationAngle_TCP")},
        {std::string("GrooveDepth"), QT_TRANSLATE_NOOP3("", "Info: groove depth [mm] of the calibration piece", "Precitec.KeyValue.GrooveDepth")},
        {std::string("GrooveWidth"), QT_TRANSLATE_NOOP3("", "Info: groove width [mm] of the calibration piece", "Precitec.KeyValue.GrooveWidth")},
        {std::string("Debug"), QT_TRANSLATE_NOOP3("", "Info: display extra log messages ", "Precitec.KeyValue.Debug")},
        {std::string("intensityAvgThreshold"), QT_TRANSLATE_NOOP3("", "parameter Threshold of filter DetectCalibrationLayers. Set to 0 for automatic guess threshold", "Precitec.KeyValue.intensityAvgThreshold")},
        {std::string("layerExtend"), QT_TRANSLATE_NOOP3("", "parameter Extent of filter DetectCalibrationLayers", "Precitec.KeyValue.layerExtend")},
        {std::string("thresholdTop"), QT_TRANSLATE_NOOP3("", "parameter Threshold of filter parallel maximum (for top layer)", "Precitec.KeyValue.thresholdTop")},
        {std::string("thresholdBot"), QT_TRANSLATE_NOOP3("", "parameter Threshold of filter parallel maximum (for bottom layer)", "Precitec.KeyValue.thresholdBot")},
        {std::string("pixPerLayer"), QT_TRANSLATE_NOOP3("", "parameter LayerSize of filter CalibrationResult", "Precitec.KeyValue.pixPerLayer")},
        {std::string("SensorModel"), QT_TRANSLATE_NOOP3("", "Calibration model currently used [0: Linear Magnification(Coax)  1: Calibration grid (Scheimpflug)]. The type of sensor must be configured in the Service page", "Precitec.KeyValue.SensorModel")},
        {std::string("scheimTriangAngle"), QT_TRANSLATE_NOOP3("", "Scheimpflug angle (between camera axis and laser plane)[deg]", "Precitec.KeyValue.scheimTriangAngle")},
        {std::string("scheimOrientationAngle"), QT_TRANSLATE_NOOP3("", "Additional angle to get the correct triangulation for a Scheimpflug system (scheimTriangAngle + scheimOrientationAngle = angle between laser plane and z axis) [deg].", "Precitec.KeyValue.scheimOrientationAngle")},
        {std::string("seamSeriesOnProductStructure"), QT_TRANSLATE_NOOP3("", "Product structure allows to create seam series", "Precitec.KeyValue.SeamSeriesOnProductStructure")},
        {std::string("seamIntervalsOnProductStructure"), QT_TRANSLATE_NOOP3("", "Product structure allows to create seam intervals for seams", "Precitec.KeyValue.SeamIntervalsOnProductStructure")},
        {std::string("configureBlackLevelOffsetVoltagesOnCamera"), QT_TRANSLATE_NOOP3("", "Camera configuration includes black level offset voltages", "Precitec.KeyValue.ConfigureBlackLevelOffsetVoltagesOnCamera")},
        {std::string("configureLinLogOnCamera"), QT_TRANSLATE_NOOP3("", "Camera configuration includes LinLog settings", "Precitec.KeyValue.ConfigureLinLogOnCamera")},
        {std::string("configureThicknessOnSeam"), QT_TRANSLATE_NOOP3("", "Seam configuration allows to configure the thickness left/right on seam", "Precitec.KeyValue.ConfigureThicknessOnSeam")},
        {std::string("configureMovingDirectionOnSeam"), QT_TRANSLATE_NOOP3("", "Seam configuration allows to configure the moving direction", "Precitec.KeyValue.ConfigureMovingDirectionOnSeam")},
        {std::string("ledCalibration"), QT_TRANSLATE_NOOP3("", "Wizard contains led calibration", "Precitec.KeyValue.LedCalibration")},
        {std::string("quickEditFilterParametersOnSeam"), QT_TRANSLATE_NOOP3("", "Seam configuration contains quick edit of filter parameters with user level operator", "Precitec.KeyValue.QuickEditFilterParametersOnSeam")},
        {std::string("qualityFaultCategory2"), QT_TRANSLATE_NOOP3("", "Error templates contain Quality Fault Category2", "Precitec.KeyValue.QualityFaultCategory2")},
        {std::string("scalePlotterFromSettings"), QT_TRANSLATE_NOOP3("", "Plotter vertical scale is determined by the values in the Result and Error Settings", "Precitec.KeyValue.ScalePlotterFromSettings")},
        {std::string("formatHardDisk"), QT_TRANSLATE_NOOP3("", "Formatting the hard disk to clean the system.", "Precitec.KeyValue.formatHardDisk")},
        {std::string("InvertX"), QT_TRANSLATE_NOOP3("", "Invert X coordinates direction (when true, higher x values [mm] are on the left of the image)", "Precitec.KeyValue.InvertX")},
        {std::string("InvertY"), QT_TRANSLATE_NOOP3("", "Invert Y coordinates direction (when true, higher y values [mm] are on the bottom of the image)", "Precitec.KeyValue.InvertX")},
        {std::string("stationId"), QT_TRANSLATE_NOOP3("", "The unique id of this Weldmaster station", "Precitec.KeyValue.stationId")},
        {std::string("stationName"), QT_TRANSLATE_NOOP3("", "Name of the Weldmaster station, allowed charaters: a-zA-Z0-9-_ maximum length: 255 characters", "Precitec.KeyValue.stationName")},
        {std::string("roleNameViewUser"), QT_TRANSLATE_NOOP3("", "The visible name of the role used by the first user", "Precitec.KeyValue.roleNameViewUser")},
        {std::string("LineLaser1Enable"), QT_TRANSLATE_NOOP3("", "Enable line laser 1", "Precitec.KeyValue.LineLaser1Enable")},
        {std::string("LineLaser2Enable"), QT_TRANSLATE_NOOP3("", "Enable line laser 2", "Precitec.KeyValue.LineLaser2Enable")},
        {std::string("FieldLight1Enable"), QT_TRANSLATE_NOOP3("", "Enable line laser 3", "Precitec.KeyValue.FieldLight1Enable")},
        {std::string("LogAllFilterProcessingTime"), QT_TRANSLATE_NOOP3("", "Enable logging of the processing time for every filter, independently from the verbosity level. If DebugTimings is enabled as well, the log messages appear in the same order as the filter processing.", "Precitec.KeyValue.LogAllFilterProcessingTime")},
        {std::string("SM_EnclosingSquareSize_mm"), QT_TRANSLATE_NOOP3("", "Enclosing square (distance between circles) [mm] in calibration target", "Precitec.KeyValue.SM_EnclosingSquareSize_mm")},
        {std::string("SM_CircleRadius_mm"), QT_TRANSLATE_NOOP3("", "Circle radius [mm] in calibration target (relevant only if SM_UseGridRecognition is off)", "Precitec.KeyValue.SM_CircleRadius_mm")},
        {std::string("SM_searchROI_X"), QT_TRANSLATE_NOOP3("", "Search ROI x (current image, after HWROI) for scanmaster calibration", "Precitec.KeyValue.SM_searchROI_X")},
        {std::string("SM_searchROI_Y"), QT_TRANSLATE_NOOP3("", "Search ROI y (current image, after HWROI) for scanmaster calibration", "Precitec.KeyValue.SM_searchROI_Y")},
        {std::string("SM_searchROI_W"), QT_TRANSLATE_NOOP3("", "Search ROI width for scanmaster calibration", "Precitec.KeyValue.SM_searchROI_W")},
        {std::string("SM_searchROI_H"), QT_TRANSLATE_NOOP3("", "Search ROI height for scanmaster calibration", "Precitec.KeyValue.SM_searchROI_H")},
        {std::string("SM_CalibrateAngle"), QT_TRANSLATE_NOOP3("", "Measure the angle between scanner and camera", "Precitec.KeyValue.SM_CalibrateAngle")},
        {std::string("SM_CalibRoutineRepetitions"), QT_TRANSLATE_NOOP3("", "Number of measurements per scanner position", "Precitec.KeyValue.SM_CalibRoutineRepetitions")},
        {std::string("WeldingDepthSystemOffset"), QT_TRANSLATE_NOOP3("", "IDM value at focus position (offset applied to the IDMWeldingDepth signal) [μm]", "Precitec.KeyValue.IDMDepthSystemOffset")},
        {std::string("colorSignalsByQuality"), QT_TRANSLATE_NOOP3("", "Color the signals observed by Errors by their quality", "Precitec.KeyValue.ColorSignalsByQuality")},
        {std::string("displayErrorBoundariesInPlotter"), QT_TRANSLATE_NOOP3("", "Display the Error Boundaries of monitored Signals in the Plotter", "Precitec.KeyValue.DisplayErrorBoundariesInPlotter")},
        {std::string("RealTimeGraphProcessing"), QT_TRANSLATE_NOOP3("", "Process graph with real time priority", "Precitec.KeyValue.RealTimeGraphProcessing")},
        {std::string("virtualKeyboard"), QT_TRANSLATE_NOOP3("", "Enable virtual keyboard for touch events (on remote desktop always enabled)", "Precitec.KeyValue.VirtualKeyboard")},
        {std::string("SensorParametersChanged"), QT_TRANSLATE_NOOP3("", "The camera parameters (e.g pixel size) are different from those used for the last calibration", "Precitec.KeyValue.SensorParametersChanged")},
        {std::string("serialNumberFromExtendedProductInfo"), QT_TRANSLATE_NOOP3("", "Use serial number from extended product info. 0 means disabled, any other value is index of field.", "Precitec.KeyValue.SerialNumberFromExtendedProductInfo")},
        {std::string("partNumberFromExtendedProductInfo"), QT_TRANSLATE_NOOP3("", "Use part number from extended product info. 0 means disabled, any other value is index of field.", "Precitec.KeyValue.PartNumberFromExtendedProductInfo")},
        {std::string("maxTimeLiveModePlotter"), QT_TRANSLATE_NOOP3("", "Maximum duration to keep values in the Plotter during live mode in seconds.", "Precitec.KeyValue.MaxTimeLiveModePlotter")},
        {std::string("remoteDesktopOnStartup"), QT_TRANSLATE_NOOP3("", "Enables remote desktop access (VNC) directly on startup.", "Precitec.KeyValue.RemoteDesktopOnStartup")},
        {std::string("blockedAutomatic"), QT_TRANSLATE_NOOP3("", "Whether the user interface is blocked for input during an automatic cycle.", "Precitec.KeyValue.BlockedAutomatic")},
        {std::string("Maximum_Simulated_Laser_Power"), QT_TRANSLATE_NOOP3("", "Maximum simulated laser power in watts [W].", "Precitec.KeyValue.MaximumSimulatedLaserPower")},
        {std::string("Laser_Power_Compensation_10us"), QT_TRANSLATE_NOOP3("", "Time to compensate time difference between laser analog reaction time and scanner tracking error. +: Laser slower than scanner -: Laser faster than scanner", "Precitec.KeyValue.Laser_Power_Compensation_10us")},
        {std::string("ScannerGeneralMode"), QT_TRANSLATE_NOOP3("", "Whether Scanner operates in ScanMaster or ScanTracker2D mode.", "Precitec.KeyValue.ScannerGeneralMode")},
        {std::string("FieldbusBoard_IP_Address"), QT_TRANSLATE_NOOP3("", "only used with Ethernet/IP", "Precitec.KeyValue.FieldbusBoard_IP_Address")},
        {std::string("FieldbusBoard_2_IP_Address"), QT_TRANSLATE_NOOP3("", "only used with Ethernet/IP", "Precitec.KeyValue.FieldbusBoard_2_IP_Address")},
        {std::string("FieldbusBoard_Netmask"), QT_TRANSLATE_NOOP3("", "only used with Ethernet/IP", "Precitec.KeyValue.FieldbusBoard_Netmask")},
        {std::string("FieldbusBoard_2_Netmask"), QT_TRANSLATE_NOOP3("", "only used with Ethernet/IP", "Precitec.KeyValue.FieldbusBoard_2_Netmask")},
    };
    const auto it = s_comments.find(key);
    if (it == s_comments.end())
    {
        return QString();
    } else
    {
        return QCoreApplication::instance()->translate("", it->second.first, it->second.second);
    }
}

}


DeviceKeyModel::DeviceKeyModel(QObject *parent)
    : AbstractDeviceKeyModel(parent)
    , m_parameterSet{new storage::ParameterSet{QUuid::createUuid(), this}}
{
    connect(this, &DeviceKeyModel::deviceProxyChanged, this, &DeviceKeyModel::init);
    connect(this, &DeviceKeyModel::notificationServerChanged, this,
        [this]
        {
            disconnect(m_notificationConnection);
            if (notificationServer())
            {
                m_notificationConnection = connect(notificationServer(), &DeviceNotificationServer::changed, this, &DeviceKeyModel::keyValueChanged, Qt::QueuedConnection);
            } else
            {
                m_notificationConnection = QMetaObject::Connection{};
            }
        }
    );
}

DeviceKeyModel::~DeviceKeyModel() = default;

QHash<int, QByteArray> DeviceKeyModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("comment")},
        {Qt::UserRole+8, QByteArrayLiteral("parameter")},
        {Qt::UserRole+9, QByteArrayLiteral("attribute")},
    };
}

QVariant DeviceKeyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    if (!deviceProxy())
    {
        return QVariant{};
    }
    if (!UserManagement::instance()->hasPermission(int(deviceProxy()->readPermission())))
    {
        return QVariant{};
    }

    const auto &kv = m_keyValues.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(kv->key());
    case Qt::UserRole:
        return commentForKey(kv->key());
    case Qt::UserRole + 8:
        return QVariant::fromValue(m_parameterSet->findByNameAndTypeId(QString::fromStdString(kv->key()), deviceProxy()->uuid()));
    case Qt::UserRole + 9:
    {
        return QVariant::fromValue(m_attributeModel ? m_attributeModel->findAttributeByName(QString::fromStdString(kv->key())) : nullptr);
    }
    }

    return QVariant{};
}

int DeviceKeyModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_keyValues.size();;
}

void DeviceKeyModel::init()
{
    beginResetModel();
    m_keyValues.clear();
    m_parameterSet->clear();
    endResetModel();

    if (!deviceProxy())
    {
        return;
    }
    setLoading(true);

    auto *watcher = new QFutureWatcher<Configuration>(this);
    connect(watcher, &QFutureWatcher<Configuration>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            beginResetModel();
            m_keyValues = watcher->result();
            for (const auto& kv : m_keyValues)
            {
                m_parameterSet->createParameter(kv, deviceProxy()->uuid());
                if (auto attribute = m_attributeModel->findAttributeByName(QString::fromStdString(kv->key())))
                {
                    // the attribute json file doesn't specify whether a keyvalue is read-only
                    attribute->setReadOnly(kv->isReadOnly());
                }
                else
                {
                    m_attributeModel->createAttribute(kv, deviceProxy()->uuid());
                }
            }
            endResetModel();
            setLoading(false);
        }
    );
    // TODO: what about the subdevices?
    watcher->setFuture(QtConcurrent::run(deviceProxy()->deviceProxy().get(), &TDevice<AbstractInterface>::get, 0));
}

void DeviceKeyModel::setLoading(bool set)
{
    if (m_loading == set)
    {
        return;
    }
    m_loading = set;
    emit loadingChanged();
}

static bool s_recursionBlocker = false;

void DeviceKeyModel::updateValue(int row, const QVariant &data)
{
    if (s_recursionBlocker)
    {
        return;
    }
    if (std::size_t(row) >= m_keyValues.size())
    {
        return;
    }
    if (!deviceProxy())
    {
        return;
    }
    if (!UserManagement::instance()->hasPermission(int(deviceProxy()->writePermission())))
    {
        return;
    }
    
    SmpKeyValue kv{m_keyValues.at(row)->clone()};   
    auto attribute = m_attributeModel->findAttributeByName(QString::fromStdString(kv->key()));
    auto val = data;
   
    if(attribute->type() == precitec::storage::Parameter::DataType::Enumeration  )
    {        
        val = attribute->convertEnumValueToInt(kv, data);
        wmLog(eInfo, "DeviceKeyModel.UpdateValue(key: %s, ComboBox Index: %d ) \n", kv->key(), data.toInt());
    }
    
    switch (kv->type())
    {
    case TInt:
       // wmLog(eInfo, "DeviceKeyMode: Update value key %s and index: %d \n", kv->key(), kv->value<int>());        
        wmLog(eInfo, "DeviceKeyMode: For index: %d converted value: %d \n", data.toInt(),  val.toInt());
        kv->setValue(val.toInt());
        break;
    case TUInt:
        kv->setValue(data.toUInt());
        break;
    case TBool:
        kv->setValue(data.toBool());
        break;
    case TFloat:
        kv->setValue(data.toFloat());
        break;
    case TDouble:
        kv->setValue(data.toDouble());
        break;
    case TString:
        kv->setValue(data.toString().toStdString());
        break;
    default:
        return;
    }
    QtConcurrent::run([this, kv] { deviceProxy()->setKeyValue(kv); });
    if (changesRequireRestart())
    {
        emit restartRequired();
    }
}

void DeviceKeyModel::keyValueChanged(const QUuid &id, const precitec::interface::SmpKeyValue &kv)
{
    if (!deviceProxy() || id != deviceProxy()->uuid() || kv.isNull())
    {
        return;
    }

    const auto it = std::find_if(m_keyValues.begin(), m_keyValues.end(), [kv] (const auto current) { return current->key() == kv->key(); });
    if (it == m_keyValues.end())
    {
        return;
    }
    *it = kv;
   // wmLog(eInfo, "DEviceKeyModel called: keyValueChanged with key: %s and value: %d \n",  kv->key(), kv->value<int>());
    // emitting dataChanged causes the gui components to update the value which might trigger calling this method again
    // thus we need a recursion blocker
    s_recursionBlocker = true;
    if (auto parameter = m_parameterSet->findByNameAndTypeId(QString::fromStdString(kv->key()), id))
    {
        switch (kv->type())
        {
        case TInt:
           // wmLog(eInfo, "DeviceKeyModel called: in Set with Int -> with key: %s and value: %d \n",  kv->key(), kv->value<int>());
            parameter->setValue(kv->value<int>());
            break;
        case TUInt:
            parameter->setValue(kv->value<uint>());
            break;
        case TBool:
            parameter->setValue(kv->value<bool>());
            break;
        case TFloat:
            parameter->setValue(kv->value<float>());
            break;
        case TDouble:
            parameter->setValue(kv->value<double>());
            break;
        case TString:
            parameter->setValue(QString::fromStdString(kv->value<std::string>()));
            break;
        case TChar:
        case TByte:
        case TNumTypes:
        case TOpMode:
        case TUnknown:
            break;
        }
    }
    const auto currentIndex{index(std::distance(m_keyValues.begin(), it), 0)};
    emit dataChanged(currentIndex, currentIndex, {Qt::UserRole + 2});
    s_recursionBlocker = false;
}

void DeviceKeyModel::setAttributeModel(storage::AttributeModel* model)
{
    if (m_attributeModel == model)
    {
        return;
    }
    disconnect(m_attributeModelDestroyed);
    m_attributeModel = model;
    if (m_attributeModel)
    {
        m_attributeModelDestroyed = connect(model, &storage::AttributeModel::destroyed, this, std::bind(&DeviceKeyModel::setAttributeModel, this, nullptr));
    }
    else
    {
        m_attributeModelDestroyed = {};
    }
    

    emit attributeModelChanged();
}

}
}
