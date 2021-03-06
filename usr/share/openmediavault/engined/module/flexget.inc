<?php
require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/initscript.inc";
require_once "openmediavault/module.inc";
require_once "openmediavault/util.inc";
class OMVModuleFlexget extends OMVModuleServiceAbstract implements
	OMVINotifyListener,
	OMVIModuleServiceStatus
{
	/**
	 * Get the main event message path of the service. This is a helper 
	 * function to avoid "magic numbers".
	 *
	 * @return string
	 */
	private function getEventMessagePath()
	{
		return "org.openmediavault.services.flexget";
	}
	
	/**
	 * Get the base XPath of the service. This is a helper function to avoid 
	 * "magic numbers".
	 *
	 * @return string
	 */
	private function getXpath()
	{
		return "/config/services/flexget";
	}
	/**
	 * Get the module name.
	 *
	 * @return string
	 */
	public function getName()
	{
		return "flexget";
	}
	
        /**
         * Stop the managed service.
         */
        public function stopService() {
                $initd = new OMVSysVInitScript($this->getName());
	        $initd->exec();
	}

        /**
         * Start the managed service.
         * @throw E_CONFIG_GET_OBJECT_FAILED
         */
        public function startService() {
                global $xmlConfig;
                
        	$object = $xmlConfig->get($this->getXPath());

        	if (is_null($object)) {
            		throw new OMVException(
                		OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                		$this->getXPath()
            		);
        	}

        	if (!boolval($object["enable"])) {
        	    return;
        	}

        	$initd = new OMVSysVInitScript($this->getName(), true);
        	$initd->exec();
	}

	/**
	 * Get the module status.
	 */
	public function getStatus()
	{
		global $xmlConfig;
		// Get the configuration object.
		$object = $xmlConfig->get($this->getXPath());
		if (is_null($object)) {
			throw new OMVException(
				OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$this->getXPath()
			);
		}
		$initd = new OMVSysVInitScript($this->getName());
		// Return the status of the service. This information is displayed
		// under Diagnostics/Services.
		return array(
			"name" => $this->getName(),
			"title" => gettext("Flexget"),
			"enabled" => boolval($object["enable"]),
			"running" => ($initd->status() == 0)
		);
	}
	/**
	 * Generate the configuration. 
	 */
	public function applyConfig()
	{
		//global $xmlConfig;
		//$cmd = sprintf("omv-mkconf %s 2>&1", $this->getName());
		$cmd = "omv-mkconf flexget 2>&1";
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(
				OMVErrorMsg::E_EXEC_FAILED,
				$cmd,
				implode("\n", $output)
			);
		}
	}
	/**
	 * Bind listeners.
	 */
	public function bindListeners(OMVNotifyDispatcher $dispatcher)
	{
		$moduleMgr = &OMVModuleMgr::getInstance();
		
		// Add listeners here. The most common thing is to monitor configuration
		// changes on the service. When the config is changed the module
		// sets itself as dirty (as seen below). Setting a module as dirty
		// makes the apply button appear in the web interface (which in turn
		// calls the applyConfig function on each module with a dirty state).
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			$this->getEventMessagePath(),
			array($this, "setDirty")
		);
	}
}
// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleFlexget());
