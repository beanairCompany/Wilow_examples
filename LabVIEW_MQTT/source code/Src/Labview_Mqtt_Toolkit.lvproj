<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="21008000">
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="Labview_Mqtt_Toolkit.vi" Type="VI" URL="../Labview_Mqtt_Toolkit.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="Check if File or Folder Exists.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Check if File or Folder Exists.vi"/>
				<Item Name="Clear Errors.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Clear Errors.vi"/>
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
				<Item Name="Get System Directory.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/sysdir.llb/Get System Directory.vi"/>
				<Item Name="LVDateTimeRec.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/miscctls.llb/LVDateTimeRec.ctl"/>
				<Item Name="NI_FileType.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/lvfile.llb/NI_FileType.lvlib"/>
				<Item Name="NI_PackedLibraryUtility.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/LVLibp/NI_PackedLibraryUtility.lvlib"/>
				<Item Name="Space Constant.vi" Type="VI" URL="/&lt;vilib&gt;/dlg_ctls.llb/Space Constant.vi"/>
				<Item Name="subTimeDelay.vi" Type="VI" URL="/&lt;vilib&gt;/express/express execution control/TimeDelayBlock.llb/subTimeDelay.vi"/>
				<Item Name="System Directory Type.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/sysdir.llb/System Directory Type.ctl"/>
				<Item Name="TCP Get Raw Net Object.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/tcp.llb/TCP Get Raw Net Object.vi"/>
				<Item Name="Trim Whitespace.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Trim Whitespace.vi"/>
				<Item Name="whitespace.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/whitespace.ctl"/>
				<Item Name="Write Delimited Spreadsheet (DBL).vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Write Delimited Spreadsheet (DBL).vi"/>
				<Item Name="Write Delimited Spreadsheet (I64).vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Write Delimited Spreadsheet (I64).vi"/>
				<Item Name="Write Delimited Spreadsheet (string).vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Write Delimited Spreadsheet (string).vi"/>
				<Item Name="Write Delimited Spreadsheet.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Write Delimited Spreadsheet.vi"/>
				<Item Name="Write Spreadsheet String.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Write Spreadsheet String.vi"/>
			</Item>
			<Item Name="Control 1.ctl" Type="VI" URL="../Control 1.ctl"/>
			<Item Name="DateTime.ISO8601.lvclass" Type="LVClass" URL="../library/Epoch Date &amp; Time/formatter.iso8601/DateTime.ISO8601.lvclass"/>
			<Item Name="DateTimeFormatter.lvclass" Type="LVClass" URL="../library/Epoch Date &amp; Time/formatter/DateTimeFormatter.lvclass"/>
			<Item Name="Epoch.GPS.lvclass" Type="LVClass" URL="../library/Epoch Date &amp; Time/epoch.GPS/Epoch.GPS.lvclass"/>
			<Item Name="Epoch.lvclass" Type="LVClass" URL="../library/Epoch Date &amp; Time/epoch/Epoch.lvclass"/>
			<Item Name="Epoch.UNIX.lvclass" Type="LVClass" URL="../library/Epoch Date &amp; Time/epoch.UNIX/Epoch.UNIX.lvclass"/>
			<Item Name="Epoch.UTC.lvclass" Type="LVClass" URL="../library/Epoch Date &amp; Time/epoch.UTC/Epoch.UTC.lvclass"/>
			<Item Name="EpochNotTimestamp.vi" Type="VI" URL="../library/Epoch Date &amp; Time/EpochNotTimestamp.vi"/>
			<Item Name="Epochs -- enum.ctl" Type="VI" URL="../library/Epoch Date &amp; Time/Epochs -- enum.ctl"/>
			<Item Name="Get Epoch Time.vi" Type="VI" URL="../library/Epoch Date &amp; Time/Get Epoch Time.vi"/>
			<Item Name="Get Local UTC Offset.vi" Type="VI" URL="../library/Epoch Date &amp; Time/Get Local UTC Offset.vi"/>
			<Item Name="GregorianCal_MonthLengths.vi" Type="VI" URL="../library/Epoch Date &amp; Time/GregorianCal_MonthLengths.vi"/>
			<Item Name="isLeapYear (integer).vi" Type="VI" URL="../library/Epoch Date &amp; Time/isLeapYear (integer).vi"/>
			<Item Name="isLeapYear (timestamp).vi" Type="VI" URL="../library/Epoch Date &amp; Time/isLeapYear (timestamp).vi"/>
			<Item Name="ISO Day -- enum.ctl" Type="VI" URL="../library/Epoch Date &amp; Time/formatter.iso8601/ISO Day -- enum.ctl"/>
			<Item Name="ISO8601 Date Formats.ctl" Type="VI" URL="../library/Epoch Date &amp; Time/formatter.iso8601/ISO8601 Date Formats.ctl"/>
			<Item Name="ISO8601 DateString to Timestamp.vi" Type="VI" URL="../library/Epoch Date &amp; Time/formatter.iso8601/ISO8601 DateString to Timestamp.vi"/>
			<Item Name="mqtt.lvclass" Type="LVClass" URL="../class/mqtt/mqtt.lvclass"/>
			<Item Name="mqttBroker.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttBroker.ctl"/>
			<Item Name="mqttCmd.lvclass" Type="LVClass" URL="../class/mqtt/mqttCmd/mqttCmd.lvclass"/>
			<Item Name="mqttCmdConnect.lvclass" Type="LVClass" URL="../class/mqtt/mqttCmd/mqttCmdConnect/mqttCmdConnect.lvclass"/>
			<Item Name="mqttCmdDisconnect.lvclass" Type="LVClass" URL="../class/mqtt/mqttCmd/mqttCmdDisconnect/mqttCmdDisconnect.lvclass"/>
			<Item Name="mqttCmdPing.lvclass" Type="LVClass" URL="../class/mqtt/mqttCmd/mqttCmdPing/mqttCmdPing.lvclass"/>
			<Item Name="mqttCmdPingResponse.vi" Type="VI" URL="../class/mqtt/mqttCmd/mqttCmdPublish/mqttCmdPingResponse.vi"/>
			<Item Name="mqttCmdPublish.lvclass" Type="LVClass" URL="../class/mqtt/mqttCmd/mqttCmdPublish/mqttCmdPublish.lvclass"/>
			<Item Name="mqttCmdPublishAckCommand.vi" Type="VI" URL="../class/mqtt/mqttCmd/mqttCmdPublish/mqttCmdPublishAckCommand.vi"/>
			<Item Name="mqttCmdPublishCompleteCommand.vi" Type="VI" URL="../class/mqtt/mqttCmd/mqttCmdPublish/mqttCmdPublishCompleteCommand.vi"/>
			<Item Name="mqttCmdPublishReceivedCommand.vi" Type="VI" URL="../class/mqtt/mqttCmd/mqttCmdPublish/mqttCmdPublishReceivedCommand.vi"/>
			<Item Name="mqttCmdSubscribe.lvclass" Type="LVClass" URL="../class/mqtt/mqttCmd/mqttCmdSubscribe/mqttCmdSubscribe.lvclass"/>
			<Item Name="mqttConnectionFlags.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttConnectionFlags.ctl"/>
			<Item Name="mqttConnectionStatus.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttConnectionStatus.ctl"/>
			<Item Name="mqttFixedHeaderResponse.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttFixedHeaderResponse.ctl"/>
			<Item Name="mqttMessageType.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttMessageType.ctl"/>
			<Item Name="mqttQOSlevel.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttQOSlevel.ctl"/>
			<Item Name="mqttRXdataCluster.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttRXdataCluster.ctl"/>
			<Item Name="mqttRXsubscriptionEvent.vi" Type="VI" URL="../class/mqtt/mqttTXqueue/mqttRXsubscriptionEvent.vi"/>
			<Item Name="mqttRXsubscriptionEventData.ctl" Type="VI" URL="../class/mqtt/mqttTXqueue/mqttRXsubscriptionEventData.ctl"/>
			<Item Name="mqttRXsubscriptionEventFunction.ctl" Type="VI" URL="../class/mqtt/mqttTXqueue/mqttRXsubscriptionEventFunction.ctl"/>
			<Item Name="mqttSubscribeCluster.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttSubscribeCluster.ctl"/>
			<Item Name="mqttTXdataQueue.vi" Type="VI" URL="../class/mqtt/mqttTXqueue/mqttTXdataQueue.vi"/>
			<Item Name="mqttTXdataQueueEnqueue.vi" Type="VI" URL="../class/mqtt/mqttTXqueue/mqttTXdataQueueEnqueue.vi"/>
			<Item Name="mqttTXdataQueueFunction.ctl" Type="VI" URL="../class/mqtt/mqttTXqueue/mqttTXdataQueueFunction.ctl"/>
			<Item Name="mqttUserInformation.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttUserInformation.ctl"/>
			<Item Name="mqttWillInformation.ctl" Type="VI" URL="../class/mqtt/mqttTypedef/mqttWillInformation.ctl"/>
			<Item Name="stringToUTF8Array.vi" Type="VI" URL="../library/string/stringToUTF8Array.vi"/>
			<Item Name="stringToUTF8ArrayWithLength.vi" Type="VI" URL="../library/string/stringToUTF8ArrayWithLength.vi"/>
			<Item Name="TCP_NoDelay_Windows.vi" Type="VI" URL="../library/tcpip/TCP_NoDelay_Windows.vi"/>
			<Item Name="Timestamp to ISO8601 Date.vi" Type="VI" URL="../library/Epoch Date &amp; Time/formatter.iso8601/Timestamp to ISO8601 Date.vi"/>
			<Item Name="Timestamp to ISO8601 Day.vi" Type="VI" URL="../library/Epoch Date &amp; Time/formatter.iso8601/Timestamp to ISO8601 Day.vi"/>
			<Item Name="Timestamp to ISO8601 UTC DateTime.vi" Type="VI" URL="../library/Epoch Date &amp; Time/formatter.iso8601/Timestamp to ISO8601 UTC DateTime.vi"/>
			<Item Name="Timestamp to ISO8601 Week.vi" Type="VI" URL="../library/Epoch Date &amp; Time/formatter.iso8601/Timestamp to ISO8601 Week.vi"/>
			<Item Name="Timestamp to ISO8601 Year.vi" Type="VI" URL="../library/Epoch Date &amp; Time/formatter.iso8601/Timestamp to ISO8601 Year.vi"/>
			<Item Name="Timestamp to Unix Time.vi" Type="VI" URL="../library/Epoch Date &amp; Time/epoch.UNIX/Timestamp to Unix Time.vi"/>
			<Item Name="Unix Time to Timestamp.vi" Type="VI" URL="../library/Epoch Date &amp; Time/epoch.UNIX/Unix Time to Timestamp.vi"/>
			<Item Name="UTC Offset to Seconds.vi" Type="VI" URL="../library/Epoch Date &amp; Time/UTC Offset to Seconds.vi"/>
			<Item Name="UTC Offsets -- enum.ctl" Type="VI" URL="../library/Epoch Date &amp; Time/UTC Offsets -- enum.ctl"/>
			<Item Name="wsock32.dll" Type="Document" URL="wsock32.dll">
				<Property Name="NI.PreserveRelativePath" Type="Bool">true</Property>
			</Item>
		</Item>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="My Application" Type="EXE">
				<Property Name="App_copyErrors" Type="Bool">true</Property>
				<Property Name="App_INI_aliasGUID" Type="Str">{5CB90952-FA87-45C3-B2D7-F7CE31849674}</Property>
				<Property Name="App_INI_GUID" Type="Str">{8F6B9E74-195B-446D-8926-4375D5461C59}</Property>
				<Property Name="App_serverConfig.httpPort" Type="Int">8002</Property>
				<Property Name="App_serverType" Type="Int">0</Property>
				<Property Name="Bld_autoIncrement" Type="Bool">true</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{1C041F0A-2459-414E-B2FC-3FE47F6B38C4}</Property>
				<Property Name="Bld_buildSpecName" Type="Str">My Application</Property>
				<Property Name="Bld_excludeInlineSubVIs" Type="Bool">true</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">../builds/NI_AB_PROJECTNAME/My Application</Property>
				<Property Name="Bld_localDestDirType" Type="Str">relativeToCommon</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{9022A928-3929-4FD9-8343-01C4CE5BC53B}</Property>
				<Property Name="Bld_version.build" Type="Int">1</Property>
				<Property Name="Bld_version.major" Type="Int">1</Property>
				<Property Name="Destination[0].destName" Type="Str">LabVIEW_MQTT_Toolkit.exe</Property>
				<Property Name="Destination[0].path" Type="Path">../builds/NI_AB_PROJECTNAME/My Application/LabVIEW_MQTT_Toolkit.exe</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">../builds/NI_AB_PROJECTNAME/My Application/data</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Source[0].itemID" Type="Str">{7FFEC080-2409-4B5A-B88D-C2166F6544C2}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref">/My Computer/Labview_Mqtt_Toolkit.vi</Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="TgtF_companyName" Type="Str">Beanair </Property>
				<Property Name="TgtF_enableDebugging" Type="Bool">true</Property>
				<Property Name="TgtF_fileDescription" Type="Str">My Application</Property>
				<Property Name="TgtF_internalName" Type="Str">Labview_mqtt_toolkit</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright © 2022 </Property>
				<Property Name="TgtF_productName" Type="Str">Labview_mqtt_toolkit</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{5A73471E-2E16-485C-BA94-B3D6F80332B2}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">LabVIEW_MQTT_Toolkit.exe</Property>
				<Property Name="TgtF_versionIndependent" Type="Bool">true</Property>
			</Item>
			<Item Name="My Installer" Type="Installer">
				<Property Name="Destination[0].name" Type="Str">Labview_Mqtt_Toolkit</Property>
				<Property Name="Destination[0].parent" Type="Str">{3912416A-D2E5-411B-AFEE-B63654D690C0}</Property>
				<Property Name="Destination[0].tag" Type="Str">{DE8EA5C1-3FC3-46E1-A277-DB9FB09EF685}</Property>
				<Property Name="Destination[0].type" Type="Str">userFolder</Property>
				<Property Name="DestinationCount" Type="Int">1</Property>
				<Property Name="DistPart[0].flavorID" Type="Str">DefaultFull</Property>
				<Property Name="DistPart[0].productID" Type="Str">{57462BB4-C3C3-4AE6-81D8-13E7A7B5C1FA}</Property>
				<Property Name="DistPart[0].productName" Type="Str">NI LabVIEW Runtime 2021 SP1 f1 (64-bit)</Property>
				<Property Name="DistPart[0].SoftDep[0].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[0].productName" Type="Str">NI ActiveX Container (64-bit)</Property>
				<Property Name="DistPart[0].SoftDep[0].upgradeCode" Type="Str">{1038A887-23E1-4289-B0BD-0C4B83C6BA21}</Property>
				<Property Name="DistPart[0].SoftDep[1].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[1].productName" Type="Str">NI Deployment Framework 2021 (64-bit)</Property>
				<Property Name="DistPart[0].SoftDep[1].upgradeCode" Type="Str">{E0D3C7F9-4512-438F-8123-E2050457972B}</Property>
				<Property Name="DistPart[0].SoftDep[10].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[10].productName" Type="Str">NI TDM Streaming 21.1</Property>
				<Property Name="DistPart[0].SoftDep[10].upgradeCode" Type="Str">{4CD11BE6-6BB7-4082-8A27-C13771BC309B}</Property>
				<Property Name="DistPart[0].SoftDep[2].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[2].productName" Type="Str">NI Error Reporting 2020 (64-bit)</Property>
				<Property Name="DistPart[0].SoftDep[2].upgradeCode" Type="Str">{785BE224-E5B2-46A5-ADCB-55C949B5C9C7}</Property>
				<Property Name="DistPart[0].SoftDep[3].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[3].productName" Type="Str">NI LabVIEW Real-Time NBFifo 2021</Property>
				<Property Name="DistPart[0].SoftDep[3].upgradeCode" Type="Str">{60862FC9-172E-3FDE-A2A4-A56A76681431}</Property>
				<Property Name="DistPart[0].SoftDep[4].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[4].productName" Type="Str">NI Logos 21.0</Property>
				<Property Name="DistPart[0].SoftDep[4].upgradeCode" Type="Str">{5E4A4CE3-4D06-11D4-8B22-006008C16337}</Property>
				<Property Name="DistPart[0].SoftDep[5].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[5].productName" Type="Str">NI LabVIEW Web Server 2021 (64-bit)</Property>
				<Property Name="DistPart[0].SoftDep[5].upgradeCode" Type="Str">{5F449D4C-83B9-492E-986B-6B85A29C431D}</Property>
				<Property Name="DistPart[0].SoftDep[6].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[6].productName" Type="Str">NI mDNS Responder 21.5</Property>
				<Property Name="DistPart[0].SoftDep[6].upgradeCode" Type="Str">{9607874B-4BB3-42CB-B450-A2F5EF60BA3B}</Property>
				<Property Name="DistPart[0].SoftDep[7].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[7].productName" Type="Str">Math Kernel Libraries 2017</Property>
				<Property Name="DistPart[0].SoftDep[7].upgradeCode" Type="Str">{699C1AC5-2CF2-4745-9674-B19536EBA8A3}</Property>
				<Property Name="DistPart[0].SoftDep[8].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[8].productName" Type="Str">Math Kernel Libraries 2020</Property>
				<Property Name="DistPart[0].SoftDep[8].upgradeCode" Type="Str">{9872BBBA-FB96-42A4-80A2-9605AC5CBCF1}</Property>
				<Property Name="DistPart[0].SoftDep[9].exclude" Type="Bool">false</Property>
				<Property Name="DistPart[0].SoftDep[9].productName" Type="Str">NI VC2015 Runtime</Property>
				<Property Name="DistPart[0].SoftDep[9].upgradeCode" Type="Str">{D42E7BAE-6589-4570-B6A3-3E28889392E7}</Property>
				<Property Name="DistPart[0].SoftDepCount" Type="Int">11</Property>
				<Property Name="DistPart[0].upgradeCode" Type="Str">{130967B8-62DA-3725-A46E-2E8360EA95EA}</Property>
				<Property Name="DistPartCount" Type="Int">1</Property>
				<Property Name="INST_autoIncrement" Type="Bool">true</Property>
				<Property Name="INST_buildLocation" Type="Path">../builds/Labview_Mqtt_Toolkit/My Installer</Property>
				<Property Name="INST_buildLocation.type" Type="Str">relativeToCommon</Property>
				<Property Name="INST_buildSpecName" Type="Str">My Installer</Property>
				<Property Name="INST_defaultDir" Type="Str">{DE8EA5C1-3FC3-46E1-A277-DB9FB09EF685}</Property>
				<Property Name="INST_installerName" Type="Str">install.exe</Property>
				<Property Name="INST_productName" Type="Str">Labview_Mqtt_Toolkit</Property>
				<Property Name="INST_productVersion" Type="Str">1.0.1</Property>
				<Property Name="InstSpecBitness" Type="Str">64-bit</Property>
				<Property Name="InstSpecVersion" Type="Str">21018001</Property>
				<Property Name="MSI_autoselectDrivers" Type="Bool">true</Property>
				<Property Name="MSI_distID" Type="Str">{B35DD53F-B266-4781-990C-90A7BF1372BF}</Property>
				<Property Name="MSI_hideNonRuntimes" Type="Bool">true</Property>
				<Property Name="MSI_osCheck" Type="Int">0</Property>
				<Property Name="MSI_upgradeCode" Type="Str">{C6BBDEE1-CA3B-4583-9C4E-F0470570345D}</Property>
				<Property Name="RegDest[0].dirName" Type="Str">Software</Property>
				<Property Name="RegDest[0].dirTag" Type="Str">{DDFAFC8B-E728-4AC8-96DE-B920EBB97A86}</Property>
				<Property Name="RegDest[0].parentTag" Type="Str">2</Property>
				<Property Name="RegDestCount" Type="Int">1</Property>
				<Property Name="Source[0].dest" Type="Str">{DE8EA5C1-3FC3-46E1-A277-DB9FB09EF685}</Property>
				<Property Name="Source[0].File[0].dest" Type="Str">{DE8EA5C1-3FC3-46E1-A277-DB9FB09EF685}</Property>
				<Property Name="Source[0].File[0].name" Type="Str">LabVIEW_MQTT_Toolkit.exe</Property>
				<Property Name="Source[0].File[0].Shortcut[0].destIndex" Type="Int">0</Property>
				<Property Name="Source[0].File[0].Shortcut[0].name" Type="Str">LabVIEW_MQTT_Toolkit</Property>
				<Property Name="Source[0].File[0].Shortcut[0].subDir" Type="Str">Labview_Mqtt_Toolkit</Property>
				<Property Name="Source[0].File[0].ShortcutCount" Type="Int">1</Property>
				<Property Name="Source[0].File[0].tag" Type="Str">{5A73471E-2E16-485C-BA94-B3D6F80332B2}</Property>
				<Property Name="Source[0].FileCount" Type="Int">1</Property>
				<Property Name="Source[0].name" Type="Str">My Application</Property>
				<Property Name="Source[0].tag" Type="Ref">/My Computer/Build Specifications/My Application</Property>
				<Property Name="Source[0].type" Type="Str">EXE</Property>
				<Property Name="SourceCount" Type="Int">1</Property>
			</Item>
		</Item>
	</Item>
</Project>
