<?php
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/notify.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/util.inc");
class OMVRpcServiceFlexget extends OMVRpcServiceAbstract
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
	 * Get the name of the RPC service.
	 *
	 * @return string
	 */
	public function getName()
	{
		return "Flexget";
	}
	/**
	 * Initialize the RPC service. The RPC methods are registered in this
	 * function with $this->registerMethod.
	 *
	 * @return void
	 */
	public function initialize()
	{
                $this->registerMethod("getSeries");
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
	}
	public function getSeries($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
				  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		OMVUtil::exec("flexget -c /home/flexget/.flexget/config.yml series list", $result);
		return implode("\n", $result);
	}
	public function getSettings($params, $context)
	{
		// $xmlConfig is needed when reading and writing from the configuration.
		global $xmlConfig;
		
		// Validate the RPC caller context.
		//
		// validateMethodContext takes the currentcontext as the first
		// parameter. The second paramter is the valid context and that can be
		// OMV_ROLE_ADMINISTRATOR, OMV_ROLE_USER or OMV_ROLE_EVERYONE.
		// This is used to make sure that the right user accesses the method.
		$this->validateMethodContext($context, array(
			"role" => OMV_ROLE_ADMINISTRATOR
		));
		
		// Get the configuration object.
		$object = $xmlConfig->get($this->getXpath());
		
		// If no data was found, throw an exception and provide the XPath that
		// failed.
		if (is_null($object)) {
			throw new OMVException(
				OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$this->getXpath()
			);
		}
		
		// Modify result data.
		// boolval and intval converts strings and numbers to their boolean
		// and integer value.
		$object["enable"] = boolval($object["enable"]);
		//$object["config"] = stringval($object["config"]);
		
		return $object;
	}
	public function setSettings($params, $context)
	{
		global $xmlConfig;
		
		$this->validateMethodContext($context, array(
			"role" => OMV_ROLE_ADMINISTRATOR
		));
		// Validate the parameters of the RPC service method.
		//
		// OpenMediavault uses JSON Schema to validate parameters. A more
		// detailed specification is provided here http://json-schema.org/
		$this->validateMethodParams(
			$params,
			'{
				"type": "object",
				"properties": {
					"enable": {
						"type": "boolean"
					},
					"config":{ 
						"type": "string"
					}
				}
			}'
		);
		// Update the configuration object.
		$object = array(
			"enable" => array_boolval($params, "enable"),
			"config" => $params["config"],
		);
		
		// Update the configuration file. If it fails it throws an exception.
		if (false === $xmlConfig->replace($this->getXpath(), $object)) {
			throw new OMVException(
				OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
				$this->getXpath()
			);
		}
		
		// Notify configuration changes.
		//
		// This will notify event listeners such as the service module
		// to perform certain tasks. The most common one is to mark the
		// service as dirty.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(
			OMV_NOTIFY_MODIFY,
			$this->getEventMessagePath(),
			$object
		);
		return $object;
	}
}
// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceFlexget());
