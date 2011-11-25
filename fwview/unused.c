	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, ULONG nValue)
	{
		OLECHAR buf[255]; 
		_snwprintf(buf , 256, L"%d", nValue);
		return pWriter->WriteElementString(NULL, pName, NULL, buf);
	}
	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, FLOAT fValue)
	{
		OLECHAR buf[255]; 
		_snwprintf(buf , 256, L"%f", fValue);
		return pWriter->WriteElementString(NULL, pName, NULL, buf);
	}
	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, bool bValue)
	{
		if (bValue) return pWriter->WriteElementString(NULL, pName, NULL, L"true");
		else return pWriter->WriteElementString(NULL, pName, NULL, L"false");
	}
	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, CBuilding::TYPE_OF_LIFT nValue)
	{
		switch (nValue)
		{
		case CBuilding::LIFT_SINGLE_DECK: return pWriter->WriteElementString(NULL, pName, NULL, L"Single Deck");
		case CBuilding::LIFT_DOUBLE_DECK: return pWriter->WriteElementString(NULL, pName, NULL, L"Double Deck");
		case CBuilding::LIFT_TWIN: return pWriter->WriteElementString(NULL, pName, NULL, L"Twin");
		default: return pWriter->WriteElementString(NULL, pName, NULL, L"Single Deck");
		}
	}
	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, CBuilding::DOOR_TYPE nValue)
	{
		switch (nValue)
		{
		case CBuilding::DOOR_CENTRE: return pWriter->WriteElementString(NULL, pName, NULL, L"Centre Opening");
		case CBuilding::DOOR_SIDE: return pWriter->WriteElementString(NULL, pName, NULL, L"Side Opening");
		default: return pWriter->WriteElementString(NULL, pName, NULL, L"Centre Opening");
		}
	}
	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, CBuilding::SHAFT_ARRANGEMENT nValue)
	{
		switch (nValue)
		{
		case CBuilding::SHAFT_INLINE: return pWriter->WriteElementString(NULL, pName, NULL, L"Inline");
		case CBuilding::SHAFT_OPPOSITE: return pWriter->WriteElementString(NULL, pName, NULL, L"Opposite");
		default: return pWriter->WriteElementString(NULL, pName, NULL, L"Inline");
		}
	}
	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, CBuilding::LOBBY_ARRANGEMENT nValue)
	{
		switch (nValue)
		{
		case CBuilding::LOBBY_THROUGH: return pWriter->WriteElementString(NULL, pName, NULL, L"Through");
		case CBuilding::LOBBY_OPENPLAN: return pWriter->WriteElementString(NULL, pName, NULL, L"Open Plan");
		default: return pWriter->WriteElementString(NULL, pName, NULL, L"Open Plan");
		}
	}
	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, CBuilding::CNTRWEIGHT_POS nValue)
	{
		switch (nValue)
		{
		case CBuilding::CNTRWEIGHT_SIDE: return pWriter->WriteElementString(NULL, pName, NULL, L"Side");
		case CBuilding::CNTRWEIGHT_REAR: return pWriter->WriteElementString(NULL, pName, NULL, L"Rear");
		default: return pWriter->WriteElementString(NULL, pName, NULL, L"Rear");
		}
	}
	HRESULT _helperWriteElementString(IXmlWriter *pWriter, LPOLESTR pName, CBuilding::CAR_ENTRANCES nValue)
	{
		switch (nValue)
		{
		case CBuilding::CAR_FRONT: return pWriter->WriteElementString(NULL, pName, NULL, L"Front");
		case CBuilding::CAR_REAR: return pWriter->WriteElementString(NULL, pName, NULL, L"Rear");
		case CBuilding::CAR_BOTH: return pWriter->WriteElementString(NULL, pName, NULL, L"Both");
		default: return pWriter->WriteElementString(NULL, pName, NULL, L"Front");
		}
	}


HRESULT CBuilding::SaveAsXML(LPOLESTR pFileName)
{
    HRESULT h;
    CComPtr<IStream> pOutFileStream;
    CComPtr<IXmlWriter> pWriter;

	FLOAT scale = GetScale();
	SetScale(1.0f);

    h = SHCreateStreamOnFile(pFileName, STGM_CREATE | STGM_WRITE, &pOutFileStream); if (FAILED(h)) return h;
    h = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL); if (FAILED(h)) return h;
    h = pWriter->SetOutput(pOutFileStream); if (FAILED(h)) return h;
    h = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE); if (FAILED(h)) return h;
    h = pWriter->WriteStartDocument(XmlStandalone_Omit); if (FAILED(h)) return h;
    h = pWriter->WriteStartElement(NULL, L"Lobby", NULL); if (FAILED(h)) return h;
		h = pWriter->WriteStartElement(NULL, L"SimulationResults", NULL); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"NoOfLifts", NoOfLifts);
			_helperWriteElementString(pWriter, L"TypeOfLift", TypeOfLift);
			_helperWriteElementString(pWriter, L"CapacityKG", CapacityKG);
			_helperWriteElementString(pWriter, L"LiftSpeed", LiftSpeed);
			_helperWriteElementString(pWriter, L"NoOfFloors", NoOfFloors);
			_helperWriteElementString(pWriter, L"FloorToFloor", FloorToFloor);
			_helperWriteElementString(pWriter, L"ExpressZone", ExpressZone);
			_helperWriteElementString(pWriter, L"FloorServed", FloorServed);
			_helperWriteElementString(pWriter, L"MT", MT);
			_helperWriteElementString(pWriter, L"UnOc", UnOc);
		h = pWriter->WriteFullEndElement(); if (FAILED(h)) return h;
		h = pWriter->WriteStartElement(NULL, L"Additional", NULL); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"DoorType", DoorType); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"DoorHeight", DoorHeight); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"DoorWidth", DoorWidth); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"CarHeight", CarHeight); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"ShaftArrangement", ShaftArrangement); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"FrontWallThickness", FrontWallThickness); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"IntDivBeam", IntDivBeam); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"LiftLobbyCHeight", LiftLobbyCHeight); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"MachRoomSlab", MachRoomSlab); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"LiftBeamHeight", LiftBeamHeight); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"PosLiftBookM", PosLiftBookM); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"NoOfBook", NoOfBook); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"PerInpTraf", PerInpTraf); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"LobbyWidth", LobbyWidth); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"LobbyArrang", LobbyArrang); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"NoOfEscalators", NoOfEscalators); if (FAILED(h)) return h;
	    h = pWriter->WriteFullEndElement(); if (FAILED(h)) return h;
		h = pWriter->WriteStartElement(NULL, L"Lift", NULL); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"CarWidth", CarWidth); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"CarDepth", CarDepth); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"ShaftWidth", ShaftWidth); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"ShaftDepth", ShaftDepth); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"CountWeight", CountWeight); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"CarEntr", CarEntr); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"PitDepth", PitDepth); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"Headroom", Headroom); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"MachRoomHeight", MachRoomHeight); if (FAILED(h)) return h;
			_helperWriteElementString(pWriter, L"OverallHeight", OverallHeight); if (FAILED(h)) return h;
	    h = pWriter->WriteFullEndElement(); if (FAILED(h)) return h;
    h = pWriter->WriteFullEndElement(); if (FAILED(h)) return h;
    h = pWriter->Flush(); if (FAILED(h)) return h;
	SetScale(scale);
	return S_OK;
}

