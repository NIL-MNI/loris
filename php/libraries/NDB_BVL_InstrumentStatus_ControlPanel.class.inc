<?php
/**
 * File contains NDB_BVL_InstrumentStatus_ControlPanel class
 *
 * PHP Version 5
 *
 * @category Main
 * @package  Behavioural
 * @author   Loris Team <loris.mni@bic.mni.mcgill.ca>
 * @license  http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 * @link     https://www.github.com/aces/Loris/
 */
require_once 'NDB_BVL_InstrumentStatus.class.inc';

/**
 * Behavioural instrument status control panel class
 *
 * This class provides the management GUI for the status flags
 * of instruments in the NeuroDB framework.
 *
 * @category Main
 * @package  Behavioural
 * @author   Loris Team <loris.mni@bic.mni.mcgill.ca>
 * @license  http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 * @link     https://www.github.com/aces/Loris/
 */
class NDB_BVL_InstrumentStatus_ControlPanel extends NDB_BVL_InstrumentStatus
{
    /**
     * Generates the HTML to display the set of buttons for the instrument
     * status flags
     *
     * @return string The HTML to render to the client
     */
    function display()
    {
        if (empty($this->_commentID)) {
            throw new Exception("No instrument instance selected");
        }

        $this->tpl_data['candID']    = isset($_REQUEST['candID'])
            ? $_REQUEST['candID']
            : '';
        $this->tpl_data['sessionID'] = $_REQUEST['sessionID'];
        $this->tpl_data['commentID'] = $_REQUEST['commentID'];
        $this->tpl_data['test_name'] = $_REQUEST['test_name'];
        if (isset($_REQUEST['subtest'])) {
            $this->tpl_data['subtest'] = $_REQUEST['subtest'];
        } else {
            $this->tpl_data['subtest'] = '';
        }

        // get the files path
        $config =& NDB_Config::singleton();
        $base   = $config->getSetting('base');

        // the instrument exists
        $InstrumentPath = $base."project/instruments/NDB_BVL_Instrument_"
            .$_REQUEST['test_name']
            .".class.inc";
        $LinstPath      = $base."project/instruments/"
            .$_REQUEST['test_name']
            .".linst";
        if (file_exists($InstrumentPath)
            || file_exists($LinstPath)
        ) {
            // include the instrument's class, if it's an old school instrument.
            if (file_exists($InstrumentPath)) {
                include_once $InstrumentPath;
            }
            // create an instance of the instrument

            $instrument =& NDB_BVL_Instrument::factory(
                $_REQUEST['test_name'],
                $_REQUEST['commentID'],
                $this->tpl_data['subtest']
            );

            $this->_instrument =& $instrument;

            $this->ValidityEnabled  = $instrument->ValidityEnabled;
            $this->ValidityRequired = $instrument->ValidityRequired;

            // generate the subtest list
            $list = $instrument->getSubtestList();
            $this->tpl_data['subtests'] = $list;
        }

        if (empty($_REQUEST['subtest'])) {
            $hasAccess = $this->_hasAccess();

            if ($this->getDataEntryStatus() == null && $hasAccess) {
                $success = $this->setDataEntryStatus('In Progress');
            }

            // generate the administration flag buttons
            $this->tpl_data['access']['administration']
                = $this->_displayAdministration();

            // generate the data entry flag buttons
            $this->tpl_data['access']['data_entry']
                = $this->_displayDataEntry();

            // generate the validity flag buttons
            if ($this->ValidityEnabled == true) {
                $this->tpl_data['access']['validity'] = $this->_displayValidity();
            }

            if ($config->getSetting('InstrumentResetting')
                && $this->getDataEntryStatus() != 'Complete'
            ) {
                $user = User::singleton();
                if ($user->hasPermission('send_to_dcc')) {
                    $this->tpl_data['InstrumentResetting'] = true;
                }
            }
        }

        $smarty = new Smarty_neurodb;
        $smarty->assign($this->tpl_data);
        $html = $smarty->fetch("instrumentstatus_controlpanel.tpl");
        return $html;
    }


    /**
     * Saves changes to the flags based on interaction with the control panel
     *
     * @return void
     */
    function save()
    {
        $hasAccess = $this->_hasAccess();
        // don't do anything if the instrument is locked
        if (!$hasAccess) {
            return false;
        }

        if (isset($_POST['ClearInstrument'])) {
            if (isset($_POST['deleteconfirm']) && isset($_POST['deleteconfirm2']) ) {
                $user = User::singleton();
                if ($user->hasPermission('send_to_dcc')) {
                    $instrument =& NDB_BVL_Instrument::factory(
                        $_REQUEST['test_name'],
                        $_REQUEST['commentID'],
                        $_REQUEST['subtest']
                    );
                    $instrument->clearInstrument();
                    return false;

                } else {
                    return false;
                    //print "No permissions to delete";
                }
            } else {
                return false;
                //print "Delete not confirmed.";
            }
            return false;
        }
        // get control panel selections from _REQUEST
        $possibleFlags = array(
                          'setDataEntry',
                          'setAdministration',
                          'setValidity',
                         );
        $flagsToSave   = array_intersect($possibleFlags, array_keys($_REQUEST));

        // make sure there are at least one flag to save
        if (is_array($flagsToSave) && count($flagsToSave) > 0) {
            foreach ($flagsToSave AS $flagToSave) {
                $method  = $flagToSave . "Status";
                $success = $this->$method($_REQUEST[$flagToSave]);
            }
        }

        return true;
    }

    /**
     * Generates the HTML to display the set of buttons for the data entry flag
     *
     * @return string
     */
    function _displayDataEntry()
    {
        $hasAccess = $this->_hasAccess();

        // loop through the options
        $i = 0;
        foreach ($this->_dataEntryOptions AS $option) {
            // skip the null case
            if (is_null($option)) {
                continue;
            }

            $this->tpl_data['data_entry'][$i]['label'] = $option;

            // highlight the current option if it's the current status
            if ($this->getDataEntryStatus() == $option) {
                $this->tpl_data['data_entry'][$i]['icon'] = 'selected';
            } elseif ($hasAccess && $this->getAdministrationStatus()) {
                // make the option a link unless this field is locked
                // the field is locked if the Administration flag has not
                // been set or the instrument is locked
                $showLink = true;
                $Validity = $this->getValidityStatus();
                if ($Validity == null
                    && $this->_instrument->ValidityRequired == true
                    && $this->getAdministrationStatus() != 'None'
                ) {
                    $showLink = false;
                }

                if ($option == 'In Progress') {
                    $showLink = true;
                } elseif ($option == 'Complete'
                    && $this->getAdministrationStatus() == 'All'
                ) {
                    // run the data entry logic test
                    $testName = Utility::getTestNameByCommentID($this->_commentID);
                    if (isset($this->_instrument)) {
                        if ($showLink == true) {
                            $CompStatus
                                = $this->_instrument->getDataEntryCompletionStatus();

                            $showLink = $CompStatus == 'Complete';
                        }
                    }
                }

                if ($showLink) {
                    // construct the URL
                    $this->tpl_data['data_entry'][$i]['showlink'] = true;
                }
            } else {
                $this->tpl_data['data_entry'][$i]['icon'] = 'locked';
            }
            $i++;
        }
        return $hasAccess;
    }

    /**
     * Generates the HTML to display the set of buttons for the administration flag
     *
     * @return string
     */
    function _displayAdministration()
    {
        $hasAccess = $this->_hasAccess();

        $dataEntry = $this->getDataEntryStatus();

        // loop through the options
        $i = 0;
        foreach ($this->_administrationOptions AS $option) {
            // skip the null case
            if (is_null($option)) {
                continue;
            }

            $this->tpl_data['administration'][$i]['label'] = $option;

            // highlight the current option if it's the current status
            if ($this->getAdministrationStatus() == $option) {
                $this->tpl_data['administration'][$i]['icon'] = 'selected';
            } elseif ($hasAccess
                && $dataEntry != 'Complete'
                && $dataEntry != 'Not Complete'
            ) {
                // make the option a link unless this field is locked
                // the field is locked if the Data_entry flag has been set
                // or the instrument is locked

                // construct the URL
                $this->tpl_data['administration'][$i]['showlink'] = true;
            } else {
                // block the button
                $this->tpl_data['administration'][$i]['icon'] = 'locked';
            }
            $i++;
        }

        return $hasAccess;
    }

    /**
     * Generates the HTML to display the set of buttons for the validity flag
     *
     * @return string
     */
    function _displayValidity()
    {
        $hasAccess = $this->_hasAccess();

        $validityStatus    = $this->getValidityStatus();
        $data_entry_status = $this->getDataEntryStatus();

        // loop through the options
        $i = 0;
        foreach ($this->_validityOptions AS $option) {
            // skip the null case
            if (is_null($option)) {
                continue;
            }

            $this->tpl_data['validity'][$i]['label'] = $option;

            // highlight the current option if it's the current status
            if ($validityStatus == $option) {
                $this->tpl_data['validity'][$i]['icon'] = 'selected';
            }
            if ($data_entry_status == 'Complete') {
                $this->tpl_data['validity'][$i]['showLink'] = false;
            } else {
                $this->tpl_data['validity'][$i]['showLink'] = true;
            }
            $i++;
        }

        return $hasAccess;
    }

    /**
     * Generates the HTML to display the set of buttons for the exclusion flag
     *
     * @return string
     */
    function _displayExclusion()
    {
        $hasAccess = $this->_hasAccess();

        $exclusionStatus = $this->getExclusionStatus();

        // loop through the options
        $i = 0;
        foreach ($this->_exclusionOptions AS $option) {
            // skip the null case
            if (is_null($option)) {
                continue;
            }

            $this->tpl_data['exclusion'][$i]['label'] = $option;

            // highlight the current option if it's the current status
            if ($exclusionStatus == $option) {
                $this->tpl_data['exclusion'][$i]['icon'] = 'selected';
            }
            $i++;
        }

        return $hasAccess;
    }

    /**
     * Determines if the current instrument statuses should be locked due
     * to being sent to dcc unless there exists feedback.
     *
     * @return boolean
     */
    function _hasAccess()
    {
        // get user object
        $user =& User::singleton();

        // make a timepoint object
        $timePoint =& TimePoint::singleton($this->getSessionID());

        // make sure user belongs to same site as timepoint
        if (!$user->hasPermission('data_entry')
            || $user->getData('CenterID') != $timePoint->getData('CenterID')
        ) {
            return false;
        }

        // make a feedback object
        $feedback =& NDB_BVL_Feedback::singleton(
            $user->getData('UserID'),
            null,
            null,
            $this->_commentID
        );

        // find out the current worst feedback thread status
        $maxThreadStatus = $feedback->getMaxThreadStatus(false);

        // statuses are locked if the timepoint is submitted and there is no
        // open feedback
        return (!$timePoint->isSubmitted() || $maxThreadStatus == 'opened');
    }
}
?>
