#include FfsERSystemAssuranceProcessor.h

// Static consts
const AmsString FfsERSystemAssuranceProcessor::BAL = "BAL";
const AmsString FfsERSystemAssuranceProcessor::NEW = "NEW";

AmsString mstrERSystemAssuranceCode;
FfsERSystemAssuranceDefinitionPtr madtERSystemAssuranceDefinition;
AmsBoolean mbDisplayDiscrepanciesOnlyFlag;
AmsBoolean mbComplexParameterEntered = FALSE;

deque <FfsERSystemAssuranceParameterGroup> madtColumnParameters;
map<AmsString, map<AmsString, AmsString>*> madtFacts1AttributeNumberCacheMap;
map<AmsString, map<AmsString, AmsString>*> madtFacts2AttributeNumberCacheMap;

AmsBoolean
FfsERSystemAssuranceProcessor::ValidateParameters()
{
	ValidateERSystemAssuranceDefinitionCode();
	ValidateDisplayDiscrepanciesOnlyFlag();
	ValidateComplexParameters();

	return IsOK();
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateComplexParameters()
{
	ValidateBalanceSheet();
	ValidateBudgetaryResources();
	ValidateChangesInNetPosition();
	ValidateCustodialActivity();
	ValidateFinancing();   
	ValidateNetCost();
	ValidateSF133();   
	ValidateFacts1Adjusted();
	ValidateFacts1Preliminary();
	ValidateFacts2Adjusted();
	ValidateFacts2Preliminary();
	ValidateGLRollup();
	ValidateComplexParameterExists();
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateERSystemAssuranceDefinitionCode()
{
	AmsString strERSystemAssuranceCode = GetParameterValue("ERSystemAssuranceCode");
	strERSystemAssuranceCode.toUpper();
	ReportParameterValue("ERSystemAssuranceCode", strERSystemAssuranceCode);

	if (strERSystemAssuranceCode.isNull())
	{
		// BA0041E - Parameter %1 is required; hence a value must be entered
		ReportProblem(AmsProblem("BA0041E") << "ERSystemAssuranceCode");
		return;
	}

	mstrERSystemAssuranceCode = strERSystemAssuranceCode;
	madtERSystemAssuranceDefinition = GetERSystemAssuranceDefinition();

	if (!madtERSystemAssuranceDefinition)
	{
		// BJ0018E: Invalid %1 specified: %2
		ReportProblem(AmsProblem("BJ0018E") << "ERSystemAssuranceCode" << strERSystemAssuranceCode);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateDisplayDiscrepanciesOnlyFlag()
{
	mbDisplayDiscrepanciesOnlyFlag = GetBooleanParameterValue ("displayDiscrepanciesOnly");
	ReportBooleanParameterValue ("displayDiscrepanciesOnly", mbDisplayDiscrepanciesOnlyFlag);

	if(mbDisplayDiscrepanciesOnlyFlag)
	{
		AmsBoolean mbTotalExists;  = FALSE;

		for(AmsInt i = 0; i < madtERSystemAssuranceDefinition.Column Count; i++)
		{
			ERSystemAssuranceDefinitionColumnPtr padtColumn = padtParentDefinition.Get Column(i);

			if(padtColumn->GetAmountsLiteralIndicator().GetValue() == FfsExternalReportAbstractDefinitionColumn::CURRENT_YEAR_TOTAL)
			{
				mbTotalExists = TRUE;
				return;
			}
		}

		if(!mbTotalExists)
		{
			// BJ2036E: The displayDiscrepanciesOnly parameter cannot be True if a Total 
			//Column does not exist on the ER System Assurance Definition specified
			ReportProblem(AmsProblem("BJ2036E"));
		}
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateComplexParameterExists()
{
	if(!mbComplexParameterEntered)
	{
		// BJ2028E: At least one Batch Group Parameter must be entered if 
		//there are values entered in the Simple Parameters
		ReportProblem(AmsProblem("BJ2028E"));
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateReportColumn(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup)
{
	AmsInt iColumnNumber = AmsStrToInteger(padtParameterGroup->GetColumnNumber())

		if(madtColumnParameters.find(iColumnNumber) != madtColumnParameters.end())
		{
			//Parameters already defined for report column %1
			AddProblem("BJ2032E") << strColumnNumber;
		}

	FfsERSystemAssuranceDefinitionColumnPtr padtColumn = padtParameterGroup->GetColumnObj();

		if(padtColumn)
		{
		if(padtParameterGroup->GetGroupName() != padtColumn.GetReportType.GetValue())
			{
				//BJ2035E Column Number %1 entered for Report Type %2 does not match a column 
				//number defined in ER System Assurance Definition %3 for the matching Report Type
			ReportProblem(AmsProblem("BJ2035E") << padtParameterGroup->GetColumnNumber() << padtParameterGroup->GetGroupName()
				<< mstrERSystemAssuranceCode);
			}

		AmsString strColumnReportCode = padtColumn->GetReportDefintionByType(padtParameterGroup->GetGroupName());

		if(!strColumnReportCode.isNull() && padtParameterGroup->GetReportCode() != strColumnReportCode)
			{
				//BJ2033E Report Code %1 entered for Batch Group %2 does not match the Report 
				//Code defined in ER System Assurance Definition %3 for Column %4
			ReportProblem(AmsProblem("BJ2033E") << padtParameterGroup->GetReportCode() << padtParameterGroup->GetGroupName()
				<< mstrERSystemAssuranceCode << padtParameterGroup->GetColumnNumber());
			}		
		}
		else
		{
			//BJ2035E Column Number %1 entered for Report Type %2 does not match a column 
			//number defined in ER System Assurance Definition %3 for the matching Report Type
		ReportProblem(AmsProblem("BJ2035E") << padtParameterGroup->GetColumnNumber() << padtParameterGroup->GetGroupName()
			<< mstrERSystemAssuranceCode);
		}
}

AmsBoolean
FfsERSystemAssuranceProcessor::ValidateParameterGroup(FfsERSystemAssuranceParmeterGroupPtr padtParameterGroup)
{
	if(!padtParameterGroup->GetColumnNumber().isNull() || !padtParameterGroup->GetFiscalMonth().isNull() ||
		!padtParameterGroup->GetFiscalQuarter().isNull() || !padtParameterGroup->GetFiscalYear().isNull() ||
		!padtParameterGroup->GetReportCode().isNull() || !padtParameterGroup->GetReportVersion().isNull() ||
		!padtParameterGroup->GetAgency().isNull()))
	{
		if(padtParameterGroup->GetColumnNumber().isNull()  || padtParameterGroup->GetFiscalYear().isNull())
		{
			if(padtParameterGroup->IsGLRollup())
			{
				if(padtParameterGroup->GetAgency().isNull())
				{
					//BJ2029E:  The following parameter values are required for the %1 group:  Agency, 
					//Fiscal Year and ER System Assurance Column Number.
					ReportProblem(AmsProblem("BJ2029E") << padtParameterGroup->GetGroupName());
				}
			}
			else
			{
				if(padtParameterGroup->GetReportCode().isNull() || padtParameterGroup->GetReportVersion().isNull())
				{
					//BJ2034E:  The following parameter values are required for the %1 group:  Report Code, 
					//Fiscal Year, Report Version and ER System Assurance Column Number.
					ReportProblem(AmsProblem("BJ2034E") << padtParameterGroup->GetGroupName());
				}
			}
		}

		if(!padtParameterGroup->GetFiscalQuarter().isNull() && !padtParameterGroup->GetFiscalMonth().isNull())
		{
			//BJ0351E:  The Fiscal Quarter and Fiscal Month cannot both be specified.
			ReportProblem(AmsProblem("BJ0351E"));
		}

		mbComplexParameterEntered = TRUE;
	}
	else
	{
		delete padtParameterGroup;
		return FALSE;
	}

	return TRUE;
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateReportParameters(const AmsString& strGroupName, AmsBaseFactory& adtFactory, AmsBaseFactory& adtDetailFactory)
{
	AmsInt iParamGroupCount = NumberOfParameterGroups(strGroupName);

	for (AmsInt iGroupNumber = 0; iGroupNumber < iParamGroupCount; iGroupNumber++) 
	{
		FfsERSystemAssuranceParameterGroupPtr padtParameterGroup = PopulateParameterGroup(strGroupName, iGroupNumber, adtFactory, adtDetailFactory);
		ValidateReportColumn(padtParameterGroup);

		if(ValidateParameterGroup(padtParameterGroup))
		{
			CheckReportExistence(padtParameterGroup, adtFactory);
		}
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateFactsReportParameters(const AmsString& strGroupName, AmsBaseFactory& adtFactory, AmsBaseFactory& adtDetailFactory)
{
	AmsInt iParamGroupCount = NumberOfParameterGroups(strGroupName);
	for (AmsInt iGroupNumber = 0; iGroupNumber < iParamGroupCount; iGroupNumber++) 
	{
		FfsERSystemAssuranceParameterGroupPtr padtParameterGroup = PopulateParameterGroup(strGroupName, iGroupNumber, adtFactory, adtDetailFactory);
		ValidateReportColumn(padtParameterGroup);

		if(ValidateParameterGroup(padtParameterGroup))
		{
			CheckFactsReportExistence(padtParameterGroup, adtFactory);
		}
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateGLRollupParameters(const AmsString& strGroupName, AmsBaseFactory& adtFactory, AmsBaseFactory& adtDetailFactory)
{
	AmsInt iParamGroupCount = NumberOfParameterGroups(strGroupName);
	for (AmsInt iGroupNumber = 0; iGroupNumber < iParamGroupCount; iGroupNumber++) 
	{
		FfsERSystemAssuranceParameterGroupPtr padtParameterGroup = PopulateGLParameterGroup(strGroupName, iGroupNumber, adtFactory, adtDetailFactory);
		ValidateReportColumn(padtParameterGroup);

		if(ValidateParameterGroup(padtParameterGroup))
		{
			madtColumnParameters[AmsStrToULong(padtParameterGroup->GetColumnNumber())] = padtParameterGroup;
		}
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateBalanceSheet()
{
	ValidateReportParameters(FfsERSystemAssuranceDefinition::BALANCE_SHEET, GetPOFactory(FfsFormAndContentBalanceSheetReport),
		GetPOFactory(FfsFormAndContentBalanceSheetReportCell));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateBudgetaryResources()
{
	ValidateReportParameters(FfsERSystemAssuranceDefinition::BUDGETARY_RESOURCES, GetPOFactory(FfsFormAndContentBudgetaryResourcesReport),
		GetPOFactory(FfsFormAndContentBudgetaryResourcesReportCell));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateChangesInNetPosition()
{
	ValidateReportParameters(FfsERSystemAssuranceDefinition::CHANGES_IN_NET_POSITION, GetPOFactory(FfsFormAndContentChangesInNetPositionReport),
		GetPOFactory(FfsFormAndContentChangesInNetPositionReportCell));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateCustodialActivity()
{
	ValidateReportParameters(FfsERSystemAssuranceDefinition::CUSTODIAL_ACTIVITY, GetPOFactory(FfsFormAndContentCustodialActivityReport),
		GetPOFactory(FfsFormAndContentCustodialActivityReportCell));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateFinancing()
{
	ValidateReportParameters(FfsERSystemAssuranceDefinition::FINANCING, GetPOFactory(FfsFormAndContentFinancingReport), 
		GetPOFactory(FfsFormAndContentFinancingReportCell));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateNetCost()
{
	ValidateReportParameters(FfsERSystemAssuranceDefinition::NET_COST, GetPOFactory(FfsFormAndContentNetCostReport),
		GetPOFactory(FfsFormAndContentNetCostReportCell));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateSF133()
{
	ValidateReportParameters(FfsERSystemAssuranceDefinition::SF133, GetPOFactory(FfsExternalReport133Report),
		GetPOFactory(FfsExternalReport133ReportCell));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateFacts1Adjusted()
{
	ValidateFactsReportParameters(FfsERSystemAssuranceDefinition::FACTS_I_ADJUSTED, GetPOFactory(FfsFacts1AdjustedTrialBalanceReport),
		GetPOFactory(FfsFacts1AdjustedTrialBalanceReportDetail));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateFacts1Preliminary()
{
	ValidateFactsReportParameters(FfsERSystemAssuranceDefinition::FACTS_I_PRELIMINARY, GetPOFactory(FfsFacts1PreliminaryTrialBalanceReport),
		GetPOFactory(FfsFacts1PreliminaryTrialBalanceReportDetail));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateFacts2Adjusted()
{
	ValidateFactsReportParameters(FfsERSystemAssuranceDefinition::FACTS_II_ADJUSTED, GetPOFactory(FfsFacts2AdjustedTrialBalanceReport),
		GetPOFactory(FfsFacts2AdjustedTrialBalanceReportDetail));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateFacts2Preliminary()
{
	ValidateFactsReportParameters(FfsERSystemAssuranceDefinition::FACTS_II_PRELIMINARY, GetPOFactory(FfsFacts2PreliminaryTrialBalanceReport),
		GetPOFactory(FfsFacts2PreliminaryTrialBalanceReportDetail));
}

AmsVoid
FfsERSystemAssuranceProcessor::ValidateGLRollup()
{
	// initially assume that we will need the factory with the lowest level of detail
	// we will recalculate this to use the most efficient factory later
	ValidateGLRollupParameters(FfsERSystemAssuranceDefinition::GL_ROLLUP, GetPOFactory(FfsGLAcctAnnualBalByVendor), GetPOFactory(FfsGLAcctAnnualBalByVendor));
}

FfsERSystemAssuranceParameterGroupPtr
FfsERSystemAssuranceProcessor::PopulateBasicParameterGroup(const AmsString& strGroupName, const AmsInt& iGroupNumber, 
														   AmsBaseFactory& adtFactory, AmsBaseFactory& adtDetailFactory)
{
	FfsERSystemAssuranceParameterGroupPtr padtParameterGroup = new FfsERSystemAssuranceParameterGroup;
	padtParameterGroup->SetGroupName(strGroupName);
	padtParameterGroup->SetFactory(&adtFactory);
	padtParameterGroup->SetDetailFactory(&adtDetailFactory);
	padtParameterGroup->SetColumnNumber(GetAndReportParameter("erSystemAssuranceColumnNumber" , strGroupName, iGroupNumber));
	padtParameterGroup->SetFiscalMonth(GetAndReportParameter("fiscalMonth", strGroupName, iGroupNumber));
	padtParameterGroup->SetFiscalQuarter(GetAndReportParameter("fiscalQuarter", strGroupName, iGroupNumber));
	padtParameterGroup->SetFiscalYear(GetAndReportParameter("fiscalYear", strGroupName, iGroupNumber));

    FfsERsystemAssuranceDefinitionColumnPtr padtColumn = madtERSystemAssuranceDefinition->GetColumn(padtParameterGroup->GetColumnNumber());
    padtParameterGroup->SetColumnId(padtColumn->GetIdentityAspect());

	return padtParameterGroup;
}

FfsERSystemAssuranceParameterGroupPtr
FfsERSystemAssuranceProcessor::PopulateParameterGroup(const AmsString& strGroupName, const AmsInt& iGroupNumber, 
													  AmsBaseFactory& adtFactory, AmsBaseFactory& adtDetailFactory)
{
	FfsERSystemAssuranceParameterGroupPtr padtParameterGroup = PopulateBasicParameterGroup(strGroupName, iGroupNumber, adtFactory, adtDetailFactory);
	padtParameterGroup->SetReportCode(GetAndReportParameter("reportCode", strGroupName, iGroupNumber));
	padtParameterGroup->SetReportVersion(GetAndReportParameter("reportVersion", strGroupName, iGroupNumber));
	return padtParameterGroup;
}

FfsERSystemAssuranceParameterGroupPtr
FfsERSystemAssuranceProcessor::PopulateGLParameterGroup(const AmsString& strGroupName, const AmsInt& iGroupNumber, AmsBaseFactory& adtFactory, AmsBaseFactory& adtDetailFactory)
{
	FfsERSystemAssuranceParameterGroupPtr padtParameterGroup = PopulateBasicParameterGroup(strGroupName, iGroupNumber, adtFactory, adtDetailFactory);
	padtParameterGroup->SetAgency(GetAndReportParameter("agency", strGroupName, iGroupNumber));
	return padtParameterGroup;
}

AmsVoid
FfsERSystemAssuranceProcessor::CheckReportExistence(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, AmsBaseFactory& adtFactory)
{
	FfsExternalReportAbstractReportPtr padtSelect = (FfsExternalReportAbstractReportPtr)adtFactory.SelectCriteriaAb();
	padtSelect->SetCode(padtParameterGroup->GetReportCode());
	padtSelect->SetVersion(padtParameterGroup->GetReportVersion());
	padtSelect->SetReportingMonth(padtParameterGroup->GetFiscalMonth());
	padtSelect->SetReportingQuarter(padtParameterGroup->GetFiscalQuarter());
	padtSelect->SetReportingYear(padtParameterGroup->GetFiscalYear());

	deque<FfsExternalReportAbstractReportPtr>* padtReportDeque =
		(deque<FfsExternalReportAbstractReportPtr>*)adtFactory.SelectAllWhereAb(padtSelect);

	delete padtSelect;

	if(!padtReportDeque->size())
	{
		//BJ2026E The Report Version for Report Type %1 does not exist, 
		//therefore no Query Records were created for %2 
		ReportProblem(AmsProblem("BJ2026E") << padtParameterGroup->GetGroupName() << mstrERSystemAssuranceCode);
		delete padtParameterGroup;
	}
	else
	{
		padtParameterGroup->SetReportId((*padtReportDeque)[0]->GetIdentityValue());
		madtColumnParameters[AmsStrToULong(padtParameterGroup->GetColumnNumber())] = padtParameterGroup;
	}

	release(padtReportDeque->begin(), padtReportDeque->end());
	delete padtReportDeque;
}

AmsVoid
FfsERSystemAssuranceProcessor::CheckFactsReportExistence(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, AmsBaseFactory& adtFactory)
{
	FfsFactsAbstractReportPtr padtSelect = (FfsFactsAbstractReportPtr)adtFactory.SelectCriteriaAb();
	padtSelect->SetCode(padtParameterGroup->GetReportCode());
	padtSelect->SetVersion(padtParameterGroup->GetReportVersion());
	padtSelect->SetReportingMonth(padtParameterGroup->GetFiscalMonth());
	padtSelect->SetReportingQuarter(padtParameterGroup->GetFiscalQuarter());
	padtSelect->SetReportingYear(padtParameterGroup->GetFiscalYear());

	deque<FfsFactsAbstractReportPtr>* padtReportDeque = (deque<FfsFactsAbstractReportPtr>*)adtFactory.SelectAllWhereAb(padtSelect);

	delete padtSelect;

	if(!padtReportDeque->size())
	{
		//BJ2026E The Report Version for Report Type %1 does not exist, 
		//therefore no Query Records were created for %2 
		ReportProblem(AmsProblem("BJ2026E") << padtParameterGroup->GetGroupName() << mstrERSystemAssuranceCode);
		delete padtParameterGroup;
	}
	else
	{
		padtParameterGroup->SetReportId((*padtReportDeque)[0]->GetIdentityValue());
		madtColumnParameters[AmsStrToULong(padtParameterGroup->GetColumnNumber())] = padtParameterGroup;
	}

	release(padtReportDeque->begin(), padtReportDeque->end());
	delete padtReportDeque;
}

AmsULong
FfsERSystemAssuranceProcessor::GenerateVersionNumber()
{
	AmsULong ulVersion;

	FfsERSystemAssuranceReportPtr padtCriteria = (FfsERSystemAssuranceReportPtr)(GetFactory().SelectCriteriaAb());
	padtCriteria->SetCode(mstrERSystemAssuranceCode);

	AmsDBSelector adtSelector;
	GetFactory().GetStorage()->SelectAllWhere(adtSelector, padtCriteria);

	AmsPartialQueryInfoPtr padtPartialQueryInfo = new AmsPartialQueryInfo;
	AmsInt iAspect = GetFactory().GetStorage()->GetColumnIndexForPartialSelect("version");
	padtPartialQueryInfo->SetQueryAspect(iAspect, AmsSQLHelper::MAX);

	AmsReaderPtr padtReader = GetFactory().GetPartialReaderWhere(adtSelector, padtPartialQueryInfo);

	if(padtReader->NextRow())
	{
		FfsERSystemAssuranceReportPtr padtResult = (FfsERSystemAssuranceReportPtr)(GetFactory().CreateSingleInstanceAb(padtReader));
		ulVersion = padtResult->GetVersion().GetValue() + 1;
		delete padtResult;
	}
	else
		ulVersion = 1;

	delete padtCriteria;
	delete padtPartialQueryInfo;
}

FfsERSystemAssuranceDefinitionPtr 
FfsERSystemAssuranceProcessor::GetERSystemAssuranceDefinition()
{
	FfsERSystemAssuranceDefinition madtERSystemAssuranceRef;
	madtERSystemAssuranceRef.SetCodeAspect(mstrERSystemAssuranceCode);
	madtERSystemAssuranceRef.AsIdentity();
	return madtERSystemAssuranceRef.GetERSystemAssuranceObj();
}

AmsString
FfsERSystemAssuranceProcessor::GetAndReportParameter(const AmsString& strParameterName, const AmsString& strGroupName, const AmsInt iGroup)
{
	AmsString strReturn = GetParameterValue( strParameterName, strGroupName, iGroup );
	ReportParameterValue(strGroupName, strParameterName, strReturn, iGroup);
	return strReturn; 
}

AmsVoid
FfsERSystemAssuranceProcessor::Process()
{
	FfsERSystemAssuranceReportPtr padtNewReport = GetPOFactory(FfsERSystemAssuranceReport).NewInstance();
	PopulateReportHeader(padtNewReport);
	PopulateReportParameters(padtNewReport);

	for(AmsInt i = 0; i < madtERSystemAssuranceDefinition->LineCount(); i++
	{
		FfsERSystemAssuranceDefinitionLinePtr padtLine =
			(FfsERSystemAssuranceDefinitionLinePtr) madtERSystemAssuranceDefinition->GetLine(i);

		if(padtLine->GetAmountsLiteralIndicator().GetValue() == FfsExternalReportAbstractDefinitionLine::AMOUNT)
		{
			map<AmsInt, AmsReaderPtr, less<AmsInt>>* padtReaderMap = GetReadersMap(padtLine);

			ProcessLine(padtReaderMap, padtNewReport, padtLine);
			release(padtReaderMap);
			delete padtReaderMap;
		}
        else
            CreateTotalsLine(padtNewReport, padtLine);
	}

	padtNewReport->RoundAmounts(madtERSystemAssuranceDefinition);

	// massage data if display discrepancies only is selected
	HandleDisplayDiscrepancies(padtNewReport);
	padtNewReport->Save();
	delete padtNewReport;
}

AmsVoid
FfsERSystemAssuranceProcessor::ProcessLine(map<AmsInt, AmsReaderPtr, less<AmsInt>>* padtReaderMap, 
										   FfsERSystemAssuranceReportPtr padtNewReport, FfsERSystemAssuranceDefinitionLinePtr padtLine)
{
	map<AmsInt, FfsERSystemAssuranceReportActivityPtr, less<AmsInt>> adtCells;

	// This method will walk through the reader map and look for a corresponding object in the adtCellsMap.
	// If none is found for the reader, it will read the next object from the reader and check to see if it matches criteria.
	// If so, it adds it to the cells map.  If not, it continues reading until it finds an eligible cell or runs out of rows.
	ReadNextCell(&adtCells, padtReaderMap, padtLine->GetLineNumber().GetValue());

	FfsERSystemAssuranceReportLinePtr padtReportLine = CreateNewReportLine(padtNewReport, padtLine);
	FfsERSystemAssuranceReportLineDetailPtr padtReportLineDetail = NULL;

	while(adtCells.size()) // there is at least one more cell to process
	{
		// Find next cell will return the cell with the lowest key values for the cell criteria.
		// It will also pull that cell out of the map. which means we own it and need to clean it up
		FfsERSystemAssuranceReportCellDetailPtr padtCell = FindNextCell(&adtCells);

		if(!padtReportLineDetail || !ReportLineMatchesCell(padtReportLineDetail, padtCell))
		{
			if(padtReportLineDetail)
			{
				padtReportLineDetail->Save();
				delete padtReportLineDetail;
				padtReportLineDetail = NULL;
			}

			padtReportLineDetail = CreateNewReportLineDetail(padtReportLine, padtCell);
		}

			padtReportLine->AddColumnAmount(padtCell->GetColumnNumber(), padtCell->GetAmount());
			padtReportLineDetail->AddColumnAmount(padtCell->GetColumnNumber(), padtCell->GetAmount());
			
		// This will add the link record needed for the drill down queries.
			AddLinkRecord(padtCell, padtReportLineDetail);

      	// Cleanup
      	delete padtCell;
      	padtCell = NULL;

		ReadNextCell(&adtCells, padtReaderMap, padtLine->GetLineNumber().GetValue());
	}

	//Save the last one
	if(padtReportLineDetail)
	{
		padtReportLineDetail->Save();
		delete padtReportLineDetail;
		padtReportLineDetail = NULL;
	}

	if(padtReportLine)
	{
		padtReport->AddLine(padtLine);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::PopulateReportHeader(FfsERSystemAssuranceReportPtr padtReport)
{
	padtReport->SetReportCode(madtERSystemAssuranceDefinition->GetCode().GetValue());
	padtReport->SetSecurityOrganizationId(madtERSystemAssuranceDefinition->GetSecurityOrganizationId().GetValue();
	padtReport->SetDisplayDiscrepanciesOnly(mbDisplayDiscrepanciesOnlyFlag);
	padtReport->SetVersion(GenerateVersionNumber());
	padtReport->SetCreationDate(AmsDate::now());
}

AmsVoid
FfsERSystemAssuranceProcessor::PopulateReportParameters(FfsERSystemAssuranceReportPtr padtReport)
{
	for(AmsInt i = 0; i < madtColumnParameters->Size(); i++)
	{
		FfsERSystemAssuranceParameterGroupPtr padtParameterGroup = (*madtColumnParameters)[i];

		FfsERSystemAssuranceReportParameterInformationPtr padtNewParameterInformation = 
			GetPOFactory(FfsERSystemAssuranceReportParameterInformation).NewInstance();
		padtNewParameterInformation->SetParentERSystemAssuranceReportId(padtReport->GetIdentityAspect().GetValue());
		padtNewParameterInformation->SetReportType(padtParameterGroup->GetGroupName());
		padtNewParameterInformation->SetColumnNumber(padtParameterGroup->GetColumnNumber());
		padtNewParameterInformation->SetReportCode(padtParameterGroup->GetReportCode());
		padtNewParameterInformation->SetReportVersion(padtParameterGroup->GetReportVersion());
		padtNewParameterInformation->SetFiscalYear(padtParameterGroup->GetFiscalYear());
		padtNewParameterInformation->SetFiscalQuarter(padtParameterGroup->GetFiscalQuarter());
		padtNewParameterInformation->SetFiscalMonth(padtParameterGroup->GetFiscalMonth());
		padtNewParameterInformation->SetAgency(padtParameterGroup->GetAgencyId());
	}
}

AmsBoolean
FfsERSystemAssuranceProcessor::ReportLineMatchesCell(FfsERSystemAssuranceReportLineDetailPtr padtDetail, FfsERSystemAssuranceReportCellDetailPtr padtCell)
{
	return (padtDetail->GetTreasurySymbolId().GetValue() == padtCell->GetTreasurySymbolId() && 
	   padtDetail->GetFundId().GetValue() == padtCell->GetFundId() &&
	   padtDetail->GetTradingPartnerId().GetValue() == padtCell->GetTradingPartnerId() &&
	   padtDetail->GetFACTSIFundGroup().GetValue() == padtCell->GetFactsFundGroup() &&
	   padtDetail->GetPartition().GetValue() == padtCell->GetPartition());
}

FfsERSystemAssuranceReportLinePtr
FfsERSystemAssuranceProcessor::CreateNewReportLine(FfsERSystemAssuranceReportPtr padtReport, FfsERSystemAssuranceDefinitionLinePtr padtLine)
{
	FfsERSystemAssuranceReportLinePtr padtNewReportLine = GetPOFactory(FfsERSystemAssuranceReportLine).NewInstance();
	padtNewReportLine->SetParentERSystemAssuranceReportId(padtReport->GetIdentityAspect().GetValue());
	padtNewReportLine->SetLineNumber(padtLine->GetLineNumber().GetValue());

	return padtNewReportLine;
}

FfsERSystemAssuranceReportLineDetailPtr
FfsERSystemAssuranceProcessor::CreateNewReportLineDetail(FfsERSystemAssuranceReportLinePtr padtReportLine, FfsERSystemAssuranceReportCellDetailPtr padtCell)
{
	FfsERSystemAssuranceReportLineDetailPtr padtNewReportLineDetail = GetPOFactory(FfsERSystemAssuranceReportLine).NewInstance();
	padtNewReportLineDetail->SetParentERSystemAssuranceReportLineId(padtReportLine->GetIdentityValue());
	padtNewReportLineDetail->SetLineNumber(padtCell->GetLineNumber());

	padtNewReportLineDetail->SetFund(padtCell->GetFund());
	padtNewReportLineDetail->SetBBFY(padtCell->GetBBFY());
	padtNewReportLineDetail->SetEBFY(padtCell->GetEBFY());
	padtNewReportLineDetail->SetPartition(padtCell->GetPartition());
	padtNewReportLineDetail->SetTreasurySymbol(padtCell->GetTreasurySymbol());
	padtNewReportLineDetail->SetFACTSIFundGroup(padtCell->GetFactsFundGroup());
	padtNewReportLineDetail->SetTradingParter(padtCell->GetTradingPartner());

	return padtNewReportLineDetail;
}

AmsVoid
FfsERSystemAssuranceProcessor::AddLinkRecord(FfsERSystemAssuranceReportCellDetailPtr padtCell, 
											 FfsERSystemAssuranceReportLineDetailPtr padtLineDetail)
{
	FfsERSystemAssuranceReportActivityPtr padtNewReportActivity = GetPOFactory(FfsERSystemAssuranceReportActivity).NewInstance();
	padtNewReportActivity->SetParentERSystemAssuranceReportLineDetailId(padtLineDetail->GetIdentityAspect().GetValue());
	padtNewReportActivity->SetColumnNumber(padtCell->GetColumnNumber().GetValue());
	padtNewReportActivity->SetReportLinkId(padtCell->GetLinkId().GetValue());

	padtNewReportActivity->Save();
	delete padtNewReportActivity;
}

AmsReaderPtr
FfsERSystemAssuranceProcessor::GetReader(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, FfsERSystemAssuranceDefinitionLinePtr padtLine, FfsERSystemAssuranceDefinitionColumnPtr padtColumn, FfsERSystemAssuranceDefinitionCellPtr padtCell)
{
	if(padtParameterGroup->IsAbstractExternalReport())
		return GetAbstractExternalReportReader(padtParameterGroup, padtLine, padtColumn, padtCell);
	else if(padtParameterGroup->IsFactsAbstractExternalReport())
		return GetFactsAbstractReportReader(padtParameterGroup, padtLine, padtColumn, padtCell);
	else if(padtParameterGroup->IsGLRollup())
		return GetGLRollupReader(padtParameterGroup, padtLine, padtColumn, padtCell);

	return NULL;
}

map<AmsInt, AmsReaderPtr, less<AmsInt>>*
FfsERSystemAssuranceProcessor::GetReadersMap(FfsERSystemAssuranceDefinitionLinePtr padtLine)
{
	map<AmsInt, AmsReaderPtr, less<AmsInt>>* padtReturn = new map<AmsInt, AmsReaderPtr, less<AmsInt>>;
	map<AmsInt, FfsERSystemAssuranceParameterGroupPtr, less<AmsInt>>::iterator it = madtColumnParameters.begin();

	for( ; it != madtColumnParameters.end(); it++)
	{
		FfsERSystemAssuranceParmeterGroupPtr padtParameterGroup = (*it).second;
		FfsERSystemAssuranceDefinitionColumnPtr padtColumn =
			madtERSystemAssuranceDefinition->GetColumn(AmsULongToStr((*it).first));
		FfsERSystemAssuranceDefinitionCellPtr padtCell =
			madtERSystemAssuranceDefinition->GetCell(padtLine->GetSectionNumber().GetValue(), padtLine->GetLineNumber().GetValue(), AmsULongToStr((*it).first));

		if(padtCell && padtColumn)
		{
			(*padtReturn)[(*it).first] = GetReader(padtParameterGroup, padtLine, padtColumn, padtCell);
		}
	}

	return padtReturn;
}

AmsReaderPtr
FfsERSystemAssuranceProcessor::GetAbstractExternalReportReader(FfsERSystemAssuranceParmeterGroupPtr padtParameterGroup, FfsERSystemAssuranceDefinitionLinePtr padtLine, FfsERSystemAssuranceDefinitionColumnPtr padtColumn, FfsERSystemAssuranceDefinitionCellPtr padtCell)
{
	AmsDBSelector adtSelector;
	FfsExternalReportAbstractReportCellPtr padtSelect =
		(FfsExternalReportAbstractReportCellPtr)padtParameterGroup->GetDetailFactory().SelectCriteriaAb();
	padtSelect->SetParentIdentity(padtParameterGroup->GetReportId());
	padtParameterGroup->GetDetailFactory().GetStorage()->SelectAllWhere(adtSelector, padtSelect);
	delete padtSelect;

	FfsExternalReportAbstractReportCellSQLPtr padtSQL = 
		(FfsExternalReportAbstractReportCellSQLPtr)padtParameterGroup->GetDetailFactory().GetStorage();
	AmsTableMapPtr padtTable = padtSQL->GetTables()->front();

	AmsManyRelationPtr padtRelation = NULL;

	// Add definition cell criteria
	if(padtParameterGroup->IsFormAndContentReport())
		padtRelation = padtCell->GetFormAndContentReportDefinitions();
	else if(padtParameterGroup->IsSF133Report())
      padtRelation = padtCell->GetExternalReportDefinitions();
		
	AddAbstractExternalReportCriteria(adtSelector, padtRelation, padtTable);

	AmsReaderPtr padtReader = padtParameterGroup->GetDetailFactory().GetNewReaderWhere(adtSelector);
	return padtReader;
}

AmsReaderPtr
FfsERSystemAssuranceProcessor::GetGLRollupReader(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup,
												 FfsERSystemAssuranceDefinitionLinePtr padtLine, FfsERSystemAssuranceDefinitionColumnPtr padtColumn, 
												 FfsERSystemAssuranceDefinitionCellPtr padtCell)
{
	// earlier we just assume FfsGLAcctAnnualBalByVendor as the factory
	// now we will figure out what is the most efficient factory that we can use
	DetermineGLFactory(padtParameterGroup, padtColumn, padtCell);
	AmsDBSelector adtSelector = GetGLRollupReaderCriteria(padtParameterGroup, padtLine, padtColumn, padtCell);
	AmsReaderPtr padtReader = padtParameterGroup->GetDetailFactory().GetNewReaderWhere(adtSelector);
	return padtReader;
}

AmsDBSelector
FfsERSystemAssuranceProcessor::GetGLRollupReaderCriteria(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup,
														 FfsERSystemAssuranceDefinitionLinePtr padtLine, FfsERSystemAssuranceDefinitionColumnPtr padtColumn, 
														 FfsERSystemAssuranceDefinitionCellPtr padtCell)
{
	AmsDBSelector adtSelector;

	FfsGLAcctBalancePtr padtSelect = (FfsGLAcctBalancePtr)padtParameterGroup->GetDetailFactory().SelectCriteriaAb();
	FfsGLAcctBalanceSQLPtr padtSQL = (FfsGLAcctBalanceSQLPtr)padtParameterGroup->GetDetailFactory().GetStorage();
	AmsTableMapPtr padtTable = padtSQL->GetTables()->front();

	padtSelect->SetAgency(padtParameterGroup->GetAgency());
	padtSelect->SetFiscalYear(padtParameterGroup->GetFiscalYear());

	if(!padtParameterGroup->GetFiscalMonth().isNull())
		padtSelect->SetFiscalMonth(padtParameterGroup->GetFiscalMonth());

	padtSQL->SelectAllWhere(adtSelector, padtSelect);
	delete padtSelect;

	if(!padtParameterGroup->GetFiscalQuarter().isNull())
	{
		AmsDBSelector adtFiscalMonthSelector = GetFiscalMonthsByQuarterSelector(padtParameterGroup->GetFiscalYear(),
			padtParameterGroup->GetFiscalQuarter());

		adtSelector.where(adtSelector.where() && padtTable->GetTable()["FISC_MNTH"].in(adtFiscalMonthSelector));
	}

	// Add definition line criteria
	AddGLFederalNonFederalCriteria(adtSelector, padtLine->GetFederalNonFederalIndicator().GetValue(), padtTable);
	AddGLRollupGLAccountCriteria(adtSelector, padtParameterGroup, padtLine->GetGLAccounts(), padtTable);
	AddTradingPartnerCriteria(adtSelector, padtParameterGroup, padtLine->GetTradingPartners(), padtTable);

	// Add definition column criteria
	AddTreasurySymbolCriteria(adtSelector, padtParameterGroup, padtColumn->GetTreasurySymbols(), padtTable);
	AddPartitionCriteria(adtSelector, padtColumn->GetPartitions(), padtTable);
	AddGLBureauCriteria(adtSelector, padtColumn->GetBureaus(), padtTable);
	AddDimensionStripCriteria(adtSelector, padtColumn->GetAccountingDimensions(), padtTable);
	AddGLFundSettingCriteria(adtSelector, padtColumn->GetFundSetting().GetValue(), padtTable);

	if(padtParameterGroup->GetFactory().GetClassID() == GetPOFactory(FfsGLAcctPeriodicBalByDist).GetClassID() ||
		padtParameterGroup->GetFactory().GetClassID() == GetPOFactory(FfsGLAcctPeriodicBalByFund).GetClassID())
	{
		AmsDBSelector adtClosingPeriodSelector = GetClosingPeriodSelector(padtParameterGroup->GetFiscalYear());
		adtSelector.where(adtSelector.where() && !padtTable->GetTable()["FISC_MNTH"].in(adtClosingPeriodSelector));
	}

	// Add definition cell criteria
	AddGLRollupGLAccountCriteria(adtSelector, padtParameterGroup, padtCell->GetGLAccounts(), padtTable);
	AddTreasurySymbolCriteria(adtSelector, padtParameterGroup, padtCell->GetTreasurySymbols(), padtTable);
	AddPartitionCriteria(adtSelector, padtCell->GetPartitions(), padtTable);
	AddGLBureauCriteria(adtSelector, padtCell->GetBureaus(), padtTable);
	AddDimensionStripCriteria(adtSelector, padtCell->GetAccountingDimensions(), padtTable);
	AddTradingPartnerCriteria(adtSelector, padtParameterGroup, padtCell->GetTradingPartners(), padtTable);

	return adtSelector;
}

AmsReaderPtr
FfsERSystemAssuranceProcessor::GetFactsAbstractReportReader(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup,
															FfsERSystemAssuranceDefinitionLinePtr padtLine, FfsERSystemAssuranceDefinitionColumnPtr padtColumn, 
															FfsERSystemAssuranceDefinitionCellPtr padtCell)
{
	AmsDBSelector adtSelector = GetFactsAbstractReportReaderCriteria(padtParameterGroup, padtLine, padtColumn, padtCell);
	AmsReaderPtr padtReader = padtParameterGroup->GetDetailFactory().GetNewReaderWhere(adtSelector);
	return padtReader;
}

AmsDBSelector
FfsERSystemAssuranceProcessor::GetFactsAbstractReportReaderCriteria(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup,
																	FfsERSystemAssuranceDefinitionLinePtr padtLine, 
																	FfsERSystemAssuranceDefinitionColumnPtr padtColumn, 
																	FfsERSystemAssuranceDefinitionCellPtr padtCell)
{
	AmsDBSelector adtSelector;
	FfsFactsAbstractReportDetailPtr padtSelect = (FfsFactsAbstractReportDetailPtr)padtParameterGroup->GetDetailFactory().SelectCriteriaAb();
	padtSelect->SetParentReportId(padtParameterGroup->GetReportId());
	padtParameterGroup->GetDetailFactory().GetStorage()->SelectAllWhere(adtSelector, padtSelect);
	delete padtSelect;

	FfsFactsAbstractReportDetailSQLPtr padtSQL = (FfsFactsAbstractReportDetailSQLPtr)padtParameterGroup->GetDetailFactory().GetStorage();
	AmsTableMapPtr padtTable = padtSQL->GetTables()->front();

	// Add definition line criteria
	AddFactsGLAccountCriteria(adtSelector, padtParameterGroup, padtLine->GetGLAccounts(), padtTable);
	AddTradingPartnerCriteria(adtSelector, padtParameterGroup, padtLine->GetTradingPartners(), padtTable);
	AddFactsFederalNonFederalCriteria(adtSelector, padtParameterGroup, padtLine->GetFederalNonFederalindicator().GetValue(), padtTable);

	// Add definition column criteria
	AddTreasurySymbolCriteria(adtSelector, padParameterGroup, padtColumn->GetTreasurySymbols(), padtTable);
	AddFactsFundCriteria(adtSelector, padtParameterGroup, padtColumn->GetAccountingDimensions(), padtTable);

	// Add definition cell criteria
	AddTreasurySymbolCriteria(adtSelector, padtParameterGroup, padtCell->GetTreasurySymbols(), padtTable);
	AddFactsFundCriteria(adtSelector, padtParameterGroup, padtCell->GetAccountingDimensions(), padtTable);
	AddTradingPartnerCriteria(adtSelector, padtParameterGroup, padtCell->GetTradingPartners(), padtTable);
	AddFactsGLAccountCriteria(adtSelector, padtParameterGroup, padtCell->GetGLAccounts(), padtTable)

	return adtSelector;
}

AmsDBCriterion
FfsERSystemAssuranceProcessor::GetGLAccountCriteria(const AmsString& strGLAccount, const AmsString& strGLRollupAccountBalance,
													const AmsString& strGLUsage, AmsTableMapPtr padtTable, const AmsString& strGLACColumn, const AmsString& strSGLColumn,
													FfsERSystemAssuranceParameterGroupPtr padtParameterGroup)
{
	AmsDBCriterion adtCriterion;
	AmsString strFilterColumn = ( !strGLACColumn.isNull() ? strGLACColumn : strSGLColumn);
	AmsBoolean bGLACId = (strGLACColumn != "GLAC");
	AmsString strSubSelectColumn = ( bGLACId ? ( !strGLACColumn.isNull() ? "UIDY" : "STND_GL_ACCT_ID") : "CD");
	AmsString strMonthColumn = "FISC_MNTH";

	AmsDBSelector adtBeginningPeriodSelector = GetBeginningPeriodSelector(padtParameterGroup->GetFiscalYear());

	if(strGLRollupAccountBalance == FfsExternalReportAbstractDefinitionCellGLAccount::BEGINNING)
	{
		adtCriterion = adtCriterion && (padtTable->GetTable()[strMonthColumn].in(adtBeginningPeriodSelector));
	}
	else if(strGLRollupAccountBalance == FfsExternalReportAbstractDefinitionCellGLAccount::CURRENT)
	{
		adtCriterion = adtCriterion && (!padtTable->GetTable()[strMonthColumn].in(adtBeginningPeriodSelector));
	}

	if(strGLUsage == FfsExternalReportAbstractDefinitionCellGLAccount::STANDARD)
	{
		if(!strSGLColumn.isNull()) // We have a STND_GL_ACCT_ID column
		{
			adtCriterion = adtCriterion && (padtTable->GetTable()[strSGLColumn] == ConvertToGLAccountId(strGLAccount, padtParameterGroup));
		}
		else // No STND_GL_ACCT_ID column so we need to use GLAC/GLAC_ID column
		{
			adtCriterion = adtCriterion && (padtTable->GetTable()[strFilterColumn].in(
				GetGLSelector(strSubSelectColumn, "STND_GL_ACCT_ID", ConvertToGLAccountId(strGLAccount, padtParameterGroup))));
		}
	}
	else if(strGLUsage == FfsExternalReportAbstractDefinitionCellGLAccount::CODED)
	{
		if(!strGLACColumn.isNull()) // We have a GLAC or GLAC_ID column
		{
			adtCriterion = adtCriterion && (padtTable->GetTable()[strGLACColumn] == ( bGLACId ? ConvertToGLAccountId(strGLAccount, padtParameterGroup) : strGLAccount) );
		}
		else // No GLAC_ID column so we need to use SGL_ACCT_ID
		{
			adtCriterion = adtCriterion && (padtTable->GetTable()[strSGLColumn].in( 
				GetGLSelector("STND_GL_ACCT_ID", "UIDY", ConvertToGLAccountId(strGLAccount, padtParameterGroup))));
		}
	}
	else if(strGLUsage == FfsExternalReportAbstractDefinitionCellGLAccount::SUMMARY)
	{
		adtCriterion = adtCriterion && (padtTable->GetTable()[strFilterColumn].in( 
			GetGLSelector(strSubSelectColumn, "SUMR_GLAC_ID", ConvertToGLAccountId(strGLAccount, padtParameterGroup))));
	}
	else if(strGLUsage == FfsExternalReportAbstractDefinitionCellGLAccount::CATEGORY)
	{
		adtCriterion = adtCriterion && (padtTable->GetTable()[strFilterColumn].in( 
			GetGLSelector( strSubSelectColumn, "ACTG_CAT_ID", strGLAccount, "FISC_YEAR", padtParameterGroup->GetFiscalYear())));
	}
	else if(strGLUsage == FfsExternalReportAbstractDefinitionCellGLAccount::CLASS)
	{
		adtCriterion = adtCriterion && (padtTable->GetTable()[strFilterColumn].in( 
			GetGLSelector( strSubSelectColumn, "ACTG_CLAS_ID", strGLAccount, "FISC_YEAR", padtParameterGroup->GetFiscalYear())));
	}
	else if(strGLUsage == FfsExternalReportAbstractDefinitionCellGLAccount::GROUP)
	{
		adtCriterion = adtCriterion && (padtTable->GetTable()[strFilterColumn].in( 
			GetGLSelector( strSubSelectColumn, "ACTG_GRP_ID", strGLAccount, "FISC_YEAR", padtParameterGroup->GetFiscalYear())));
	}
	else if(strGLUsage == FfsExternalReportAbstractDefinitionCellGLAccount::TYPE)
	{
		adtCriterion = adtCriterion && (padtTable->GetTable()[strFilterColumn].in( 
			GetGLSelector( strSubSelectColumn, "ACTG_TYP_ID", strGLAccount, "FISC_YEAR", padtParameterGroup->GetFiscalYear())));
	}

	return adtCriterion;
}

AmsVoid
FfsERSystemAssuranceProcessor::AddGLFederalNonFederalCriteria(AmsDBSelector& adtSelector, 
															  const AmsString& strFederalNonFederalIndicator, AmsTableMapPtr padtTable)
{
	if(!strFederalNonFederalIndicator.isNull())
	{
		if(strFederalNonFederalIndicator == FEDERAL)
		{ 
			adtSelector.where(adtSelector.where() && (padtTable->GetTable()["TRDG_PTNR_TYP"] == "F" ||
				padtTable->GetTable()["FCT1_FDRL_IN"] == "F"));
		}
		else
		{
			// Translate non federal to X/E for Trading Partner Type and N for Facts I Federal Indicator
			adtSelector.where(adtSelector.where() && (padtTable->GetTable()["TRDG_PTNR_TYP"] == "E" ||
				padtTable->GetTable()["TRDG_PTNR_TYP"] == "X" || padtTable->GetTable()["FCT1_FDRL_IN"] == "N"));
		}
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddGLTransferTSYMCriteria(AmsDBSelector& adtSelector, 
														 FfsExternalReportAbstractDefinitionCellTradingPartnerPtr padtTradingPartner, AmsTableMapPtr padtTable)
{
	deque<AmsString> adtIncludeDeque;
	deque<AmsString> adtExcludeDeque;

    AmsDBCriterion adtCriterion;
    adtCriterion = padtTable->GetTable()["TRDG_PTNR"].isNull();
    adtSelector.where(adtSelector.where() || adtCriterion);
    AmsBoolean bInclude = padtTradingPartner->GetIncludeExcludeIndicator().GetValue();

    AmsDBSelector adtTSYMSelector;
    FfsTreasurySymbolSQLPtr padtTreasurySymbolSQL = (FfsTreasurySymbolSQLPtr).GetPOFactory(FfsTreasurySymbol).GetStorage();
    AmsDBTable adtTreasurySymbolTable = padtTreasurySymbolSQL->GetTables()->front()->GetTable();
    adtTSYMSelector << adtTreasurySymbolTable["UIDY"];
    adtTSYMSelector.where(adtSelector.where() && (adtTreasurySymbolTable["S133_AGCY_ID"] = padtTradingPartner->GetTradingPartnerId().GetValue() || 
       adtTreasurySymbolTable["SRCE_AGCY_ID"] == padtTradingPartner->GetTradingPartnerId().GetValue()));
    AmsReaderPtr padtReader = (FfsTreasurySymbolPtr)GetPOFactory(FfsTreasurySymbol).GetNewReaderWhere(adtSelector);

    if(padtReader)
    {
        while(padtReader->NextRow())
        {
           AmsString strIdentity;
           (*padtReader) >> strIdentity;

           if(bInclude)
              adtIncludeDeque.push_back(strIdentity);
	else
              adtExcludeDeque.push_back(strIdentity);
        }
    }

	AddCriterionToSelector(adtSelector, padtTable->GetTable()["TRFR_TSYM_ID"],
		adtIncludeDeque, adtExcludeDeque);
}

AmsVoid
FfsERSystemAssuranceProcessor::AddPartitionCriteria(AmsDBSelector& adtSelector,
													AmsManyRelationPtr padtPartitions, AmsTableMapPtr padtTable)
{
	if(padtPartitions->Size())
	{
		deque<AmsString> adtIncludeDeque;
		deque<AmsString> adtExcludeDeque;

		for(AmsInt i = 0; i < padtPartitions->Size(); i++)
		{
			FfsExternalReportAbstractDefinitionPartitionPtr padtPartition = (*padtPartitions)[i];

			adtIncludeDeque.push_back(padtPartition->GetPartition().GetValue());
		}

		AddCriterionToSelector(adtSelector, padtTable->GetTable()["PATN"], adtIncludeDeque, adtExcludeDeque);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddGLBureauCriteria(AmsDBSelector& adtSelector, 
												   AmsManyRelationPtr padtBureaus, AmsTableMapPtr padtTable)
{

	if(padtBureaus->Size())
	{
		deque<AmsDBCriterion> adtIncludeDeque;
		deque<AmsDBCriterion> adtExcludeDeque;

		for(AmsInt i = 0; i < padtBureaus->Size(); i++
		{
			FfsExternalReportAbstractDefinitionBureauPtr padtBureau = (*padtBureaus)[i];
			AddFundBureauCriterion(padtBureau->GetBureauId().GetValue(), adtIncludeDeque, TRUE, padtTable);
		}

		AddCriterionToSelector(adtSelector, adtIncludeDeque, adtExcludeDeque);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddDimensionStripCriteria(AmsDBSelector& adtSelector
														 AmsManyRelationPtr padtDimensionStrips, AmsTableMapPtr padtTable)
{
	if(padtDimensionStrips->Size())
	{
		deque<AmsCriterion> adtIncludeDeque;
		deque<AmsCriterion> adtExcludeDeque;

		for(AmsInt i = 0; i < padtDimensionStrips->Size(); i++)
		{
			FfsExternalReportAbstractDefinitionDimensionStripPtr padtDimStrip = (*padtDimensionStrips)[i];

			if(padtDimStrip->GetIncludeExcludeIndicator().GetValue() == FfsExternalReportAbstractDefinitionCell::INCLUDE)
				AddDimensionStripCriterion(padtDimStrip, adtIncludeDeque, TRUE, padtTable);
			else
				AddDimensionStripCriterion(padtDimStrip, adtExcludeDeque, FALSE, padtTable);
		}

		AddCriterionToSelector(adtSelector, adtIncludeDeque, adtExcludeDeque);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddGLFactsAttributeCriteria(AmsDBCriterion & adtCriterion, 
														   AmsManyRelationPtr padtFacts1Attributes, AmsManyRelationPtr padtFacts2Attributes,
														   AmsTableMapPtr padtTable, const AmsBoolean& bInclude)
{
	if(padtFacts1Attributes->Size())
	{
		AmsDBCriterion adtAttributeCriterion;

		for(AmsInt i = 0; i < padtFacts1Attributes->Size(); i++)
		{

			FFSERSystemAssuranceDefinitionCellGLFACTSPtr padtGLFacts1 = padtGLAccount->GetFacts1Attribute(i);

			FfsFACTSAttributeDefinitionReference adtAttrRef;
			adtAttrRef.SetAttributeNumber( padtGLFacts1->GetAttributeNumber().GetValue());
			adtAttrRef.SetFiscalYear( padtParameterGroup->GetFiscalYear() );

			FfsFACTSAttributeDefinitionPtr padtAttribute = (FfsFACTSAttributeDefinitionPtr) adtAttrRef.GetAttributeDefinitionObj();

			if(padtAttribute->GetFederalNonFederalFlag().GetValue())
			{
				AddToSubCriterion(adtAttributeCriterion, padtTable->GetTable()["FCT1_FDRL_IN"], padtGLFacts1->GetDomainValue().GetValue(), bInclude);
			}
		}

		if(bInclude)
			adtCriterion = adtCriterion && (adtAttributeCriterion);
		else
			adtCriterion = adtCriterion || (adtAttributeCriterion);
	}

	if(padtFacts2Attributes->Size())
	{
		AmsDBCriterion adtAttributeCriterion;

		for(AmsInt i = 0; i < padtFacts2Attributes->Size(); i++)
		{
			FFSERSystemAssuranceDefinitionCellGLFACTS2Ptr padtGLFacts2 = padtGLAccount.GetFacts2Attribute(i);

			FfsFACTS2AttributeDefinitionReference adtAttrRef;
			adtAttrRef.SetAttributeNumber( padtGLFacts2->GetAttributeNumber().GetValue());
			adtAttrRef.SetFiscalYear( padtParameterGroup->GetFiscalYear() );

			FfsFACTS2AttributeDefinitionPtr padtAttribute = (FfsFACTS2AttributeDefinitionPtr) adtAttrRef.GetAttributeDefinitionObj();

			if(padtAttribute->GetYBAFlag().GetValue()
			{
               if(padtGLFacts2->GetDomainValue().GetValue() == FfsERSystemAssuranceProcessor::NEW)
				{
					if(bInclude)
						adtAttributeCriterion = adtAttributeCriterion && padtTable->GetTable()["YBA"] >= padtParameterGroup->GetFiscalYear();
					else
						adtAttributeCriterion = adtAttributeCriterion || padtTable->GetTable()["YBA"] < padtParameterGroup->GetFiscalYear();
				}
               else if (padtGLFacts2->GetDomainValue().GetValue() == FfsERSystemAssuranceProcessor::BAL)
				{
					if(bInclude)
						adtAttributeCriterion = adtAttributeCriterion && padtTable->GetTable()["YBA"] < padtParameterGroup->GetFiscalYear();
					else
						adtAttributeCriterion = adtAttributeCriterion || padtTable->GetTable()["YBA"] >= padtParameterGroup->GetFiscalYear();
				}
			}

			if(padtAttribute->GetTransactionPartnerFlag().GetValue())
			{
				AddToSubCriterion(adtAttributeCriterion, padtTable->GetTable()["TRDG_PTNR_TYP"], padtGLFacts2->GetDomainValue().GetValue(), bInclude);
			}

			if(padtAttribute->GetPriorYearAdjustmentsFlag().GetValue())
			{
				AddToSubCriterion(adtAttributeCriterion, padtTable->GetTable()["PRYR_ADJM"], padtGLFacts2->GetDomainValue().GetValue(), bInclude);
			}

			if(padtAttribute->GetProgramReportingCategoryFlag())
			{
				AddToSubCriterion(adtAttributeCriterion, padtTable->GetTable()["PRC"], padtGLFacts2->GetDomainValue().GetValue(), bInclude);
			}

			if(padtAttribute->GetPublicLawFlag().GetValue())
			{
				AddToSubCriterion(adtAttributeCriterion, padtTable->GetTable()["PBLC_LAW_NUM"], padtGLFacts2->GetDomainValue().GetValue(), bInclude);
			}
		}

		if(bInclude)
			adtCriterion = adtCriterion && (adtAttributeCriterion);
		else
			adtCriterion = adtCriterion || (adtAttributeCriterion);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddToSubCriterion(AmsDBCriterion& adtCriterion, AmsDBColumn& adtColumn, const AmsString& strValue, const AmsBoolean& bInclude)
{
	if(bInclude)
		adtCriterion = adtCriterion || adtColumn == strValue;
	else
		adtCriterion = adtCriterion && adtColumn != strValue;
}

AmsVoid
FfsERSystemAssuranceProcessor::AddToCriterion(AmsDBCriterion& adtCriterion, AmsDBColumn& adtColumn, const AmsString& strValue, const AmsBoolean& bInclude)
{
	if(bInclude)
		adtCriterion = adtCriterion && adtColumn == strValue;
	else
		adtCriterion = adtCriterion || adtColumn != strValue;
}

AmsVoid
FfsERSystemAssuranceProcessor::AddGLFundSettingCriteria(AmsDBSelector& adtSelector, 
														const AmsString& strFundSetting, AmsTableMapPtr padtTable)
{
	deque<AmsDBCriterion> adtIncludeDeque;
	deque<AmsDBCriterion> adtExcludeDeque;

	if(strFundSetting == FfsERSystemAssuranceDefinitionColumn::FACTS1) 
		AddFundFactsInclusionCriterion("T", "F", adtIncludeDeque, TRUE, padtTable);
	else if(strFundSetting == FfsERSystemAssuranceDefinitionColumn::FACTS2)
		AddFundFactsInclusionCriterion("F", "T", adtIncludeDeque, TRUE, padtTable);
	else if(strFundSetting == FfsERSystemAssuranceDefinitionColumn::FACTS1_FACTS2)
		AddFundFactsInclusionCriterion("T", "T", adtIncludeDeque, TRUE, padtTable);

	AddCriterionToSelector(adtSelector, adtIncludeDeque, adtExcludeDeque);
}

AmsVoid
FfsERSystemAssuranceProcessor::AddGLRollupGLAccountCriteria(AmsDBSelector& adtSelector, 
															FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, AmsManyRelationPtr padtGLAccounts, AmsTableMapPtr padtTable)
{
	if(padtGLAccounts->Size())
	{
		AmsString strGLACColumn = "GLAC";
		deque<AmsDBCriterion> adtIncludeDeque;
		deque<AmsDBCriterion> adtExcludeDeque;

		for(AmsInt i = 0; i < padtGLAccounts->Size(); i++)
		{
			// Create GL account criteria
			FfsERSystemAssuranceDefinitionCellGLAccountPtr padtGLAccount = (*padtGLAccounts)[i];
			AmsBoolean bInclude = padtGLAccount->GetIncludeExcludeIndicator().GetValue();
			AmsString strGLAccountCriteria = padtGLAccount->GetGLAccountCriteriaByUsage();
			AmsDBCriterion adtCriterion = GetGLAccountCriteria(strGLAccountCriteria, "", padtGLAccount->GetGLUsageIndicator.GetValue(),
				padtTable, strGLACColumn, "", padtParameterGroup);

			if(!bInclude)
				adtCriterion = !(adtCriterion);

			// Add facts attribute criteria
			AddGLFactsAttributeCriteria(adtCriterion, padtGLAccount->GetFacts1Attributes(), padtGLAccount->GetFacts2Attributes(),
				padtTable, bInclude);

			AmsDBSelector adtBeginningPeriodSelector = GetBeginningPeriodSelector(padtParameterGroup->GetFiscalYear());

			if(padtGLAccount->GetGLRollupAcctBalanceIndicator().GetValue() == FfsExternalReportAbstractDefinitionCellGLAccount::BEGINNING)
			{
				if(bInclude)
				   adtCriterion = adtCriterion && (padtTable->GetTable()["FISC_MNTH"].in(adtBeginningPeriodSelector));
                else
                   adtCriterion = adtCriterion || !(padtTable->GetTable()["FISC_MNTH"].in(adtBeginningPeriodSelector));
			}
			else if(padtGLAccount->GetGLRollupAcctBalanceIndicator().GetValue() == FfsExternalReportAbstractDefinitionCellGLAccount::CURRENT)
			{
				if(bInclude)
				adtCriterion = adtCriterion && !(padtTable->GetTable()["FISC_MNTH"].in(adtBeginningPeriodSelector));
                else
                   adtCriterion = adtCriterion || (padtTable->GetTable()["FISC_MNTH"].in(adtBeginningPeriodSelector));
			}

			// Add to the include/exclude criteria list
			if(bInclude)
				adtIncludeDeque.push_back(adtCriterion);
			else
				adtExcludeDeque.push_back(adtCriterion);
		}

		AddCriterionToSelector(adtSelector, adtIncludeDeque, adtExcludeDeque);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddFactsGLAccountCriteria(AmsDBSelector& adtSelector, FfsERSystemAssuranceParameterGroupPtr padtParameterGroup,
														 AmsManyRelationPtr padtGLAccounts, AmsTableMapPtr padtTable)
{
	if(padtGLAccounts->Size())
	{
		AmsString strGLACColumn = (padtParameterGroup->IsFactsPreliminaryReport() ? "GLAC_ID" : "");
		AmsString strSGLColumn = "SGL_ACCT_ID";
		deque<AmsDBCriterion> adtIncludeDeque;
		deque<AmsDBCriterion> adtExcludeDeque;

		for(AmsInt i = 0; i < padtGLAccounts->Size(); i++)
		{
			// Create GL account criteria
			FfsERSystemAssuranceDefinitionCellGLAccountPtr padtGLAccount = (*padtGLAccounts)[i];
			AmsBoolean bInclude = padtGLAccount->GetIncludeExcludeIndicator().GetValue();
			AmsString strGLAccountCriteria = padtGLAccount->GetGLAccountCriteriaByUsage();
			AmsDBCriterion adtCriterion = GetGLAccountCriteria(strGLAccountCriteria, "", padtGLAccount->GetGLUsageIndicator.GetValue(),
				padtTable, strGLACColumn, strSGLColumn, padtParameterGroup);

			if(!bInclude)
				adtCriterion = !(adtCriterion);


			// Add begin/end criteria
			if(!padtGLAccount->GetFactsIIBeginEndIndicator().GetValue().isNull() && padtParameterGroup->IsFacts2Report())
			{
				AddToCriterion(adtCriterion, padtTable->GetTable()["BEGN_END_IN"], padtGLAccount->GetFactsIIBeginEndIndicator().GetValue(), bInclude);
			}

			// Add facts attribute criteria
			AddFactsAttributeCriteria(adtCriterion, padtParameterGroup, (padtParameterGroup->IsFacts1Report() ? 
				padtGLAccount->GetFacts1Attributes() : padtGLAccount->GetFacts2Attributes()), padtTable, bInclude);

			// Add to the include/exclude criteria list
			if(bInclude)
				adtIncludeDeque.push_back(adtCriterion);
			else
				adtExcludeDeque.push_back(adtCriterion);
		}

		AddCriterionToSelector(adtSelector, adtIncludeDeque, adtExcludeDeque);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddFactsAttributeCriteria(AmsDBCriterion & adtCriterion,
														 FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, AmsManyRelationPtr padtAttributes, 
														 AmsTableMapPtr padtTable, const AmsBoolean& bInclude)
{
	if(padtAttributes->Size())
	{
		AmsDBCriterion adtAttributeCriterion;

		for(AmsInt i = 0; i < padtAttributes->Size(); i++)
		{
			FfsExternalReportAbstractCellGLAccountFACTS1AttributeValue padtGLFacts1;
			FfsExternalReportAbstractCellGLAccountFACTS2AttributeValue padtGLFacts2;

			if(padtParameterGroup->IsFacts1Report)
				padtGLFacts1 = (*padtAttributes)[i];
			else
				padtGLFacts2 = (*padtAttributes)[i];

			AmsString strAttributeNumber = (padtGLFacts1 ? padtGLFacts1->GetAttributeNumber().GetValue() : 
				padtGLFacts2->GetAttributeNumber().GetValue());

			// Remove leading zero if necessary
			strAttributeNumber = (strAttributeNumber(0,1) == "0" ? strAttributeNumber(1) : strAttributeNumber);

			AmsString strDomainValue = (padtGLFacts1 ? padtGLFacts1->GetDomainValue().GetValue() : 
				padtGLFacts2->GetDomainValue().GetValue());

			AddToSubCriterion(adtCriterion, padtTable->GetTable()["ATTR_" + strAttributeNumber + "_VAL"], strDomainValue, bInclude);
		}

		if(bInclude)
			adtCriterion = adtCriterion && (adtAttributeCriterion);
		else
			adtCriterion = adtCriterion || (adtAttributeCriterion);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddTradingPartnerCriteria(AmsDBSelector& adtSelector, 
														 FfsERSystemAssuranceParameterGroupPtr padtParameterGroup,
														 AmsManyRelationPtr padtTradingPartners, AmsTableMapPtr padtTable)
{
	if(padtTradingPartners->Size())
	{
		AmsString strTradingPartnerAttributeNumber = NULL;

		if(padtParameterGroup->IsFactsAbstractExternalReport())
		{
			strTradingPartnerAttributeNumber = GetFactsAttributeNumber(
				(padtParameterGroup->IsFacts1Report() ? "TRDG_PTNR_AGCY_FL" : "TRFR_AGCY_ACCT_FL"), padtParameterGroup);
		}

		deque<AmsString> adtIncludeDeque;
		deque<AmsString> adtExcludeDeque;

		for(AmsInt i = 0; i < padtTradingPartners->Size(); i++
		{
			FfsExternalReportAbstractDefinitionCellTradingPartnerPtr padtPartner = (*padtTradingPartners)[i];

			if(padtPartner->GetIncludeExcludeIndicator().GetValue() == FfsExternalReportAbstractDefinitionCell::INCLUDE)
				adtIncludeDeque.push_back(padtPartner->GetTradingPartner().GetValue());
			else
				adtExcludeDeque.push_back(padtPartner->GetTradingPartner().GetValue());
		}

		if(padtParameterGroup->IsFactsAbstractExternalReport())
		{
			AddCriterionToSelector(adtSelector, padtTable->GetTable()["ATTR_" + strTradingPartnerAttributeNumber + "_VAL"],
				adtIncludeDeque, adtExcludeDeque);
		}
		else
		{
			AddCriterionToSelector(adtSelector, padtTable->GetTable()["TRDG_PTNR"], adtIncludeDeque, adtExcludeDeque);
		}

        if(padtParameterGroup->IsGLRollup)
           AddGLTransferTSYMCriteria(adtSelector, padtPartner, padtTable);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddFactsFederalNonFederalCriteria(AmsDBSelector& adtSelector, 
																 FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, const AmsString& strFederalNonFederalIndicator, AmsTableMapPtr padtTable)
{
	if(!strFederalNonFederalIndicator.isNull())
	{
		AmsString strFederalNonFederalAttributeNumber = GetFactsAttributeNumber(
			(padtParameterGroup->IsFacts1Report() ? "TRDG_PTNR_FL" : "TRAN_PTNR_FL"), padtParameterGroup);

		if(strFederalNonFederalIndicator == NON_FEDERAL && padtParameterGroup->IsFacts2Report())
		{ // Translate non federal to X/E
			adtSelector.where(adtSelector.where() && 
				(padtTable->GetTable()["ATTR_" + strFederalNonFederalAttributeNumber + "_VAL"] == "X" ||
				padtTable->GetTable()["ATTR_" + strFederalNonFederalAttributeNumber + "_VAL"] == "E"));
		}
		else // otherwise just use the value directly
		{
			adtSelector.where(adtSelector.where() && padtTable->GetTable()["ATTR_" + strFederalNonFederalAttributeNumber + "_VAL"] ==
				strFederalNonFederalIndicator);
		}
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddTreasurySymbolCriteria(AmsDBSelector& adtSelector,
														 FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, AmsManyRelationPtr padtTreasurySymbols, AmsTableMapPtr padtTable)
{
	if(padtTreasurySymbols->Size())
	{
		AmsString strColumn;

		if(padtParameterGroup->IsGLRollup() || padtParameterGroup->IsFacts2Report())
			strColumn = "TSYM";
		else
			strColumn = "FT_FUND_GRP";

		deque<AmsString> adtIncludeDeque;
		deque<AmsString> adtExcludeDeque;

		for(AmsInt i = 0; i < padtTreasurySymbols->Size(); i++)
		{
			AmsString strValue;
			FfsExternalReportAbstractDefinitionTreasurySymbolPtr padtTSYM = (*padtTreasurySymbols)[i];
			AmsBoolean bInclude = padtTSYM->GetIncludeExcludeIndicator().GetValue();

			if(padtParameterGroup->IsFacts1Report())
				strValue = padtTSYM->GetFactsFundGroup().GetValue();

			if(padtParameterGroup->IsGLRollup() || padtParameterGroup->IsFacts2Report())
			{
				if(bInclude)
					AddTreasurySymbolCriterion(padtTSYM, padtParameterGroup, adtIncludeDeque, bInclude, padtTable);
				else
					AddTreasurySymbolCriterion(padtTSYM, padtParameterGroup, adtExcludeDeque, bInclude, padtTable);
			}
			else // FACTS I report
			{
				if(bInclude)
					adtIncludeDeque.push_back(strValue);
				else
					adtExcludeDeque.push_back(strValue);
			}
		}

		AddCriterionToSelector(adtSelector, padtTable->GetTable()[strColumn], adtIncludeDeque, adtExcludeDeque);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddFactsFundCriteria(AmsDBSelector& adtSelector, FfsERSystemAssuranceParameterGroupPtr padtParameterGroup,
														 AmsManyRelationPtr padtDimensionStrips, AmsTableMapPtr padtTable)
{
	if(padtDimensionStrips->Size())
	{
		deque<AmsDBCriterion> adtIncludeDeque;
		deque<AmsDBCriterion> adtExcludeDeque;

		for(AmsInt i = 0; i < padtDimensionStrips->Size(); i++)
		{
			FfsExternalReportAbstractDefinitionDimensionStripPtr padtDimStrip = (*padtDimensionStrips)[i];
			AmsBoolean bInclude = padtDimensionStrip->GetIncludeExcludeIndicator().GetValue();

			// Add fund criterion
			if(bInclude)
				AddFactsFundCriterion(padtDimStrip, adtIncludeDeque, bInclude, padtTable);
			else
				AddFactsFundCriterion(padtDimStrip, adtExcludeDeque, bInclude, padtTable);
		}

		AddCriterionToSelector(adtSelector, adtIncludeDeque, adtExcludeDeque);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddAbstractExternalReportCriteria(AmsDBSelector& adtSelector,
														 AmsManyRelationPtr padtAbstractExternalReportDefinitions, AmsTableMapPtr padtTable)
{
	if(padtAbstractExternalReportDefinitions->Size())
	{
		AmsDBCriterion adtCriterion;
		deque<AmsCriterion> adtIncludeDeque;
		deque<AmsCriterion> adtExcludeDeque;

		for(AmsInt i = 0; i < padtAbstractExternalReportDefinitions->Size(); i++)
		{
			FfsERSystemAssuranceDefinitionCellAbstractExternalReportDefinitionPtr padtDefinition = (FfsERSystemAssuranceDefinitionCellAbstractExternalReportDefinitionPtr)(*padtAbstractExternalReportDefinitions)[i];

			adtCriterion = padtTable->GetTable()["SEC_NUM"] == padtDefinition->GetSectionNumber().GetValue() &&
				padtTable->GetTable()["LNUM"] == padtDefinition->GetLineNumber().GetValue() && padtTable->GetTable()["COLM_NUM"] ==
				padtDefinition->GetColumnNumber().GetValue() && padtTable->GetTable()["TYP"] == "";

			adtIncludeDeque.push_back(adtCriterion);
		}

		AddCriterionToSelector(adtSelector, adtIncludeDeque, adtExcludeDeque);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddTreasurySymbolCriterion(FfsExternalReportAbstractDefinitionTreasurySymbolPtr padtTreasurySymbol,
														  FfsERSystemAssuranceParameterGroup padtParameterGroup,
														  deque<AmsString> &adtDeque, const AmsBoolean& bInclude, 
														  AmsTableMapPtr padtTable)
{
	AmsDBSelector adtSelector;
	FfsTreasurySymbolSQLPtr padtTreasurySymbolSQL = (FfsTreasurySymbolSQLPtr).GetPOFactory(FfsTreasurySymbol).GetStorage();
	AmsDBTable adtTreasurySymbolTable = padtTreasurySymbolSQL->GetTables()->front()->GetTable();

	if(padtParameterGroup->IsFacts2Report())
		adtSelector << adtTreasurySymbolTable["UIDY"];
	else
		adtSelector << adtTreasurySymbolTable["TSYM"];

	if(padtTreasurySymbol->GetAgency().GetValue())
	{
		adtSelector.where(adtSelector.where() && adtTreasurySymbolTable["S133_AGCY_ID"] == 
			padtTreasurySymbol->GetAgency().GetValue());
	}

	if(padtTreasurySymbol->GetSourceAgency().GetValue())
	{
		adtSelector.where(adtSelector.where() && adtTreasurySymbolTable["SRCE_AGCY_ID"] == 
			padtTreasurySymbol->GetSourceAgency().GetValue());
	}

	if(padtTreasurySymbol->GetAvailableFromYear().GetValue())
	{
		adtSelector.where(adtSelector.where() && adtTreasurySymbolTable["AVAL_FROM_YEAR"] ==
			padtTreasurySymbol->GetAvailableFromYear().GetValue());
	}

	if(padtTreasurySymbol->GetAvailableToYear().GetValue())
	{
		adtSelector.where(adtSelector.where() && adtTreasurySymbolTable["AVAL_TO_YEAR"] ==
			padtTreasurySymbol->GetAvailableToYear().GetValue());
	}

	if(padtTreasurySymbol->GetAvailabilityType().GetValue())
	{
		adtSelector.where(adtSelector.where() && adtTreasurySymbolTable["AVAL_TYP"] ==
			padtTreasurySymbol->GetAvailabilityType().GetValue());
	}

	if(padtTreasurySymbol->GetFundSymbol().GetValue())
	{
		adtSelector.where(adtSelector.where() && adtTreasurySymbolTable["FUND_SYMB"] ==
			padtTreasurySymbol->GetFundSymbol().GetValue());
	}

	if(padtTreasurySymbol->GetSubAccount().GetValue())
	{
		adtSelector.where(adtSelector.where() && adtTreasurySymbolTable["SUB_ACCT"] ==
			padtTreasurySymbol->GetSubAccount().GetValue());.
	}

	AmsGenericReaderPtr padtReader  = new AmsGenericReader(adtSelector, (AmsDBReadOnlyConnectionPtr) NULL);
	AmsString strTSYMIdentifier;

	if(padtReader)
	{
		while(padtReader->NextRow())
		{
			(*padtReader) >> strTSYMIdentifier;
			AmsDBCriterion adtCriterion;

			if(padtParameterGroup->IsFacts2Report())
				AddToCriterion(adtCriterion, padtTable->GetTable()["TSYM_ID"], strTSYMIdentifier, bInclude);
			else
				AddToCriterion(adtCriterion, padtTable->GetTable()["TSYM"], strTSYMIdentifier, bInclude);

			adtDeque.push_back(adtCriterion);
		}
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddFactsFundCriterion(FfsExternalReportAbstractDefinitionDimensionStripPtr padtDimensionStrip,
													 deque<AmsString> &adtDeque, const AmsBoolean& bInclude, AmsTableMapPtr padtTable)
{
	AmsDBSelector adtSelector;
	FfsFundSQLPtr padtFundSQL = (FfsFundSQLPtr).GetPOFactory(FfsFund).GetStorage();
	AmsDBTable adtFundTable = padtFundSQL->GetTables()->front()->GetTable();
	
	adtSelector << adtFundTable["UIDY"];
	
	if(padtDimensionStrip->GetFund().GetValue())
		adtSelector.where(adtSelector.where() && adtFundTable["CD"] == padtDimensionStrip->GetFund().GetValue());

	if(padtDimensionStrip->GetBegBudgetFY().GetValue())
		adtSelector.where(adtSelector.where() && adtFundTable["BBFY"] == padtDimensionStrip->GetBegBudgetFY().GetValue());

	if(padtDimensionStrip->GetEndBudgetFY().GetValue().isNull())
    {
        if(bInclude)
		    adtSelector.where(adtSelector.where() && adtFundTable["EBFY"].isNull());
	else
            adtSelector.where(adtSelector.where() || !adtFundTable["EBFY"].isNull());
    }
	else
		adtSelector.where(adtSelector.where() && adtFundTable["EBFY"] == padtDimensionStrip->GetEndBudgetFY().GetValue());

	if(padtDimensionStrip->GetPartitionId().GetValue())
		adtSelector.where(adtSelector.where() && adtFundTable["PATN_ID"] == padtDimensionStrip->GetPartitionId().GetValue());

	AmsGenericReaderPtr padtReader  = new AmsGenericReader(adtSelector, (AmsDBReadOnlyConnectionPtr) NULL);
	AmsString strIdentity;
	
	if(padtReader)
	{
		while(padtReader->NextRow())
		{
			(*padtReader) >> strIdentity;
			AmsDBCriterion adtCriterion;
			
			AddToCriterion(adtCriterion, padtTable->GetTable()["FUND_ID"], strIdentity, bInclude);

			adtDeque.push_back(adtCriterion);
		}
	}
}

AmsString
FfsERSystemAssuranceProcessor::GetFactsAttributeNumber(const AmsString& strColumn, FfsERSystemAssuranceParameterGroupPtr padtParameterGroup)
{
	if(padtParameterGroup->IsFacts1Report())
	{
		return GetFactsAttributeNumber(&(GetPOFactory(FfsFACTSAttributeDefinition)), madtFacts1AttributeNumberCacheMap, 
			strColumn, padtParameterGroup->GetFiscalYear());
	}
	else
	{
		return GetFactsAttributeNumber(&(GetPOFactory(FfsFACTS2AttributeDefinition)), madtFacts2AttributeNumberCacheMap, 
			strColumn, padtParameterGroup->GetFiscalYear()));
	}
}

AmsString
FfsERSystemAssuranceProcessor::GetFactsAttributeNumber(AmsBaseFactoryPtr padtFactory, 
													   map< AmsString, map<AmsString, AmsString>* >& adtCacheMap, const AmsString& strColumn, const AmsString& strFiscalYear)
{
	// Look in the cache first
	map< AmsString, map<AmsString, AmsString>* >::iterator it =  adtCacheMap.find(strFiscalYear);

	if(it != adtCacheMap.end())
	{
		map<AmsString, AmsString>::iterator itColumn = (*it).second->find(strColumn);

		if(itColumn != (*it).second->end())
		{
			return (*itColumn).second;
		}
	}

	// Didn't find it in the cache, so we need to look in the db
	AmsDBSelector adtSelector;
	AmsPersistentStoragePtr padtSQL = padtFactory->GetStorage();
	AmsTableMapPtr padtTable = padtSQL->GetTables()->front();

	adtSelector << padtTable->GetTable()["ATTR_NUM"];
	adtSelector.where(adtSelector.where() && padtTable->GetTable()["FISC_YEAR"] = strFiscalYear);
	adtSelector.where(adtSelector.where() && padtTable->GetTable()[strColumn] = "T");

	AmsString strAttributeNumber;
	AmsGenericReaderPtr padtReader  = new AmsGenericReader(adtSelector, (AmsDBReadOnlyConnectionPtr) NULL);

	if (padtReader->NextRow())
	{
		*padtReader >> strAttributeNumber;
	}

	// Remove leading zero if necessary
	strAttributeNumber = (strAttributeNumber(0,1) == "0" ? strAttributeNumber(1) : strAttributeNumber);

	// Put the result in cache
	if(it == adtCacheMap.end())// need to create the map for the fiscal year
	{
		adtCacheMap[strFiscalYear] = new map<String, String>;
		it = adtCacheMap.find(strFiscalYear)
	}

	(*(*it).second)[strColumn] = strAttributeNumber;
	return strAttributeNumber;
}

AmsVoid
FfsERSystemAssuranceProcessor::AddCriterionToSelector(AmsDBSelector& adtSelector, const AmsDBColumn &adtColumn,
													  deque<AmsString> &adtIncludeDeque, deque<AmsString> &adtExcludeDeque)
{
	if(adtIncludeDeque.size())
	{
		AmsDBCriterion adtIncludeCriterion = AmsSQLHelper::BuildInClauseForStringDeque(adtColumn, adtIncludeDeque, FALSE, FALSE);
		adtSelector.where( adtSelector.where() && ( adtIncludeCriterion ) );
	}

	if(adtExcludeDeque.size())
	{
		AmsDBCriterion adtExcludeCriterion = AmsSQLHelper::BuildInClauseForStringDeque(adtColumn, adtExcludeDeque, FALSE, TRUE);
		adtSelector.where( adtSelector.where() && ( adtExcludeCriterion ) );
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::AddCriterionToSelector(AmsDBSelector& adtSelector, deque<AmsDBCriterion> &adtIncludeDeque, deque<AmsDBCriterion> &adtExcludeDeque)
{
	if(adtIncludeDeque.size())
	{
		AmsDBCriterion adtIncludeCriterion;

		for(AmsInt i = 0; i < adtIncludeDeque.size(); i++)
		{
			AmsDBCriterion adtTempCriterion = adtIncludeDeque[i];
			adtIncludeCriterion = adtIncludeCriterion || ( adtTempCriterion );
		}

		adtSelector.where( adtSelector.where() && ( adtIncludeCriterion ) );
	}

	if(adtExcludeDeque.size())
	{
		AmsDBCriterion adtExcludeCriterion;

		for(AmsInt i = 0; i < adtExcludeDeque.size(); i++)
		{
			AmsDBCriterion adtTempCriterion = adtExcludeDeque[i];
			adtExcludeCriterion = adtExcludeCriterion && ( adtTempCriterion );
		}

		adtSelector.where( adtSelector.where() && ( adtExcludeCriterion ) );
	}
}

AmsString
FfsERSystemAssuranceProcessor::ConvertToGLAccountId(const AmsString& strGLAccount, FfsERSystemAssuranceParameterGroupPtr padtParameterGroup)
{
	FfsGLAccountReference adtGLAccountRef;  
	AmsStringAspect adtFiscalYear;
	adtGLAccountRef.ConnectTo( adtFiscalYear );
	adtGLAccountRef.SetCodeAspect( strGLAccount );
	adtFiscalYear.SetValue( padtParameterGroup->GetFiscalYear() );
	adtGLAccountRef.AsIdentity()

		return adtGLAccountRef.GetIdentityAspect().GetValue();
}

AmsDBSelector
FfsERSystemAssuranceProcessor::GetGLSelector(const AmsString& strSelectedColumn, const AmsString& strSelectedByColumn1, const AmsString& strValue1,
											 const AmsString& strSelectedByColumn2, const AmsString& strValue2)
{
	AmsDBSelector adtReturnSelector; 
	FfsGLAccountSQLPtr padtGLSQL = (FfsGLAccountSQLPtr).GetPOFactory(FfsGLAccount).GetStorage();
	AmsDBTable adtGLTable = padtGLSQL->GetTables()->front()->GetTable();
	adtReturnSelector << adtGLTable[strSelectedColumn];
	adtReturnSelector.where(adtReturnSelector.where() && adtGLTable[strSelectedByColumn1] = strValue1);

	if(!strSelectedByColumn2.isNull())
	{
		adtReturnSelector.where(adtReturnSelector.where() && adtGLTable[strSelectedByColumn2] = strValue2);
	}

	return adtReturnSelector;
}

AmsDBSelector
FfsERSystemAssuranceProcessor::GetClosingPeriodSelector(const AmsString& strFiscalYear)
{
	AmsDBSelector adtReturnSelector;
	FfsAccountingPeriodSQLPtr padtAccountingPeriodSQL = (FfsAccountingPeriodSQLPtr).GetPOFactory(FfsAccountingPeriod).GetStorage();
	AmsDBTable adtAccountingPeriodTable = padtAccountingPeriodSQL->GetTables()->front()->GetTable();
	adtReturnSelector << adtAccountingPeriodTable["FISC_MNTH"];
	adtReturnSelector.where(adtReturnSelector.where() && adtAccountingPeriodTable["CLSG_PERD_FL"] == "T" 
		&& adtAccountingPeriodTable["FISC_YEAR"] == strFiscalYear);
	return adtReturnSelector;
}

AmsDBSelector
FfsERSystemAssuranceProcessor::GetBeginningPeriodSelector(const AmsString& strFiscalYear)
{
	AmsDBSelector adtReturnSelector;
	FfsAccountingPeriodSQLPtr padtAccountingPeriodSQL = (FfsAccountingPeriodSQLPtr).GetPOFactory(FfsAccountingPeriod).GetStorage();
	AmsDBTable adtAccountingPeriodTable = padtAccountingPeriodSQL->GetTables()->front()->GetTable();
	adtReturnSelector << adtAccountingPeriodTable["FISC_MNTH"];
	adtReturnSelector.where(adtReturnSelector.where() && adtAccountingPeriodTable["BEGN_PERD_FL"] == "T"
		&& adtAccountingPeriodTable["FISC_YEAR"] == strFiscalYear);
	return adtReturnSelector;
}

AmsDBSelector
FfsERSystemAssuranceProcessor::GetFiscalMonthsByQuarterSelector(const AmsString& strFiscalYear, const AmsString& strFiscalQuarter)
{
	AmsDBSelector adtReturnSelector;
	FfsAccountingPeriodSQLPtr padtAccountingPeriodSQL = (FfsAccountingPeriodSQLPtr).GetPOFactory(FfsAccountingPeriod).GetStorage();
	AmsDBTable adtAccountingPeriodTable = padtAccountingPeriodSQL->GetTables()->front()->GetTable();
	adtReturnSelector << adtAccountingPeriodTable["FISC_MNTH"];
	adtReturnSelector.where(adtReturnSelector.where() && adtAccountingPeriodTable["FISC_YEAR"] == strFiscalYear
		&& adtAccountingPeriodTable["FISC_QUAR"] == strFiscalQuarter);
	return adtReturnSelector;
}

FfsERSystemAssuranceReportCellDetailPtr
FfsERSystemAssuranceProcessor::FindNextCell(map<AmsInt, FfsERSystemAssuranceReportCellDetailPtr, less<AmsInt>> adtCellsMap)
{
	FfsERSystemAssuranceReportCellDetailPtr padtReturn = NULL;
	AmsInt iReader = -1;

	map<AmsInt, FfsERSystemAssuranceReportCellDetailPtr, less<AmsInt>>::iterator it = adtCellsMap.begin();

	for( ; it != padtReaderMap.end(); it++)
	{
		FfsERSystemAssuranceReportCellDetailPtr padtCellDetail = (*it).second;

		if(padtReturn == NULL || (*padtCellDetail) < (*padtReturn))
		{
			padtReturn = padtCellDetail;
			iReader = (*it).first;
		}
	}

	it = adtCellsMap.find(iReader);
	adtCellsMap.erase(it);
	return padtReturn;
}

AmsVoid
FfsERSystemAssuranceProcessor::ReadNextCell(map<AmsInt, FfsERSystemAssuranceReportCellDetailPtr, less<AmsInt>> adtCells, map<AmsInt, AmsReaderPtr, less<AmsInt>>* padtReaderMap, AmsString strLineNumber)
{
	map<AmsInt, AmsReaderPtr, less<AmsInt>>::iterator it = padtReaderMap.begin();

	for( ; it != padtReaderMap.end(); it++)
	{
		AmsInt iReader = (*it).first;
		map<AmsInt, AmsReaderPtr, less<AmsInt>>::iterator itCell = adtCells.find(iReader);

		if(itCell == adtCells.end() || (*itCell).second == NULL)
		{
			AmsReaderPtr padtReader = (*it).second;

			if(padtReader && padtReader->NextRow())
			{
				map<AmsInt, FfsERSystemAssuranceParameterGroupPtr, less<AmsInt>>::iterator itParam = madtColumnParameters.find(iReader);
				FfsERSystemAssuranceParameterGroupPtr padtParameterGroup = (*itParam).second;
				adtCells[iReader] = CreateReportCellDetail(padtParameterGroup, padtReader, strLineNumber);
			}
		}
	}
}

FfsERSystemAssuranceReportCellDetailPtr
FfsERSystemAssuranceProcessor::CreateReportCellDetail(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, AmsReaderPtr padtReader, AmsString strLineNumber)
{
	if(padtParameterGroup->IsAbstractExternalReport())
		return CreateAbstractExternalReportCellDetail(padtParameterGroup, padtReader, strLineNumber);
	else if(padtParameterGroup->IsFactsAbstractExternalReport())
		return CreateFactsAbstractReportCellDetail(padtParameterGroup, padtReader, strLineNumber);
	else if(padtParameterGroup->IsGLRollup())
		return CreateGLRollupCellDetail(padtParameterGroup, padtReader, strLineNumber);
}

AmsVoid
FfsERSystemAssuranceProcessor::AddFundBureauCriterion(const AmsString& strBureauId, deque<AmsString> &adtDeque, const AmsBoolean& bInclude, AmsTableMapPtr padtTable)
{
	FfsFundSQLPtr padtFundSQL = (FfsFundSQLPtr).GetPOFactory(FfsFund).GetStorage();
	AmsDBTable adtFundTable = padtFundSQL->GetTables()->front()->GetTable();
	AmsDBCriterion adtCriterion;
	adtCriterion = (adtFundTable["BURU_ID"] = strBureauId);
	AddFundCriterion(adtCriterion, adtDeque, bInclude, padtTable);
}

AmsVoid
FfsERSystemAssuranceProcessor::AddFundFactsInclusionCriterion(const AmsString& strFacts1, const AmsString& strFacts2, 
															  deque<AmsString> &adtDeque, const AmsBoolean& bInclude, AmsTableMapPtr padtTable)
{
	FfsFundSQLPtr padtFundSQL = (FfsFundSQLPtr).GetPOFactory(FfsFund).GetStorage();
	AmsDBTable adtFundTable = padtFundSQL->GetTables()->front()->GetTable();
	AmsDBCriterion adtCriterion;
	adtCriterion = (adtFundTable["FCTS_INCL_FL"] == strFacts1 && adtFundTable["FCT2_INCL_FL"] == strFacts2);
	AddFundCriterion(adtCriterion, adtDeque, bInclude, padtTable);
}

AmsVoid
FfsERSystemAssuranceProcessor::AddFundCriterion(AmsDBCriterion& adtCriterion, deque<AmsString> &adtDeque, const AmsBoolean& bInclude, AmsTableMapPtr padtTable)
{
	AmsDBSelector adtSelector;
	FfsFundSQLPtr padtFundSQL = (FfsFundSQLPtr).GetPOFactory(FfsFund).GetStorage();
	AmsDBTable adtFundTable = padtFundSQL->GetTables()->front()->GetTable();
	adtSelector << adtFundTable["CD"];
	adtSelector << adtFundTable["BBFY"];
	adtSelector << adtFundTable["EBFY"];
	adtSelector << adtFundTable["PATN_ID"];
	adtSelector.where(adtSelector.where() && adtCriterion);
	AmsGenericReaderPtr padtReader  = new AmsGenericReader(adtSelector, (AmsDBReadOnlyConnectionPtr) NULL);

	AmsDBColumn adtBBFYColumn = padtTable->GetTable()["BBFY"];
	AmsDBColumn adtEBFYColumn = padtTable->GetTable()["EBFY"];
	AmsDBColumn adtFundColumn = padtTable->GetTable()["FUND"];
	AmsDBColumn adtPartitionColumn = padtTable->GetTable()["PATN"];

	AmsString strFund, strBBFY, strEBFY, strPartitionId;

	if(padtReader)
	{
		while(padtReader->NextRow())
		{
			(*padtReader) >> strFund;
			(*padtReader) >> strBBFY;
			(*padtReader) >> strEBFY;
			(*padtReader) >> strPartitionId;

			AmsDBCriterion adtTempCriterion;

			AddToCriterion(adtTempCriterion, adtFundColumn, strFund, bInclude);
			AddToCriterion(adtTempCriterion, adtBBFYColumn, strBBFY, bInclude);

			if(strEBFY.isNull())
			{
				if(bInclude)
					adtTempCriterion = adtTempCriterion && adtEBFYColumn.isNull();
				else
					adtTempCriterion = adtTempCriterion || !adtEBFYColumn.isNull();
			}
			else
				AddToCriterion(adtTempCriterion, adtEBFYColumn, strEBFY, bInclude);

			if(!strPartitionId.isNull())
				AddToCriterion(adtTempCriterion, adtPartitionColumn, ConvertToPartitionCode(strPartitionId), bInclude);

			adtDeque.push_back(adtTempCriterion);
		}
	}
}

FfsERSystemAssuranceReportCellDetailPtr
FfsERSystemAssuranceProcessor::CreateAbstractExternalReportCellDetail(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, 
																	  AmsReaderPtr padtReader, AmsString strLineNumber)
{
	FfsExternalReportAbstractReportCellPtr padtCell = (FfsExternalReportAbstractReportCellPtr)(padtParameterGroup->GetDetailFactory().CreateSingleInstanceAb(padtReader));
	FfsERSystemAssuranceReportCellDetailPtr padtCellDetail = NULL;

	if(padtCell)
	{
		FfsERSystemAssuranceDefinitionColumnPtr padtColumn = padtParameterGroup->GetColumnObj();

		FfsExternalReportAbstractReportReference adtReport;
		adtReport.SetIdentityAspect(padtCell->GetParentIdentity().GetValue());
		adtReport.AsIdentity();
		FfsExternalReportAbstractReportPtr padtReport = adtReport.GetReportObj();

		padtCellDetail->SetLineNumber(strLineNumber);
		padtCellDetail->SetColumnNumber(padtParameterGroup->GetColumnNumber());

		if(padtReport)
			padtCellDetail->SetPartition(padtReport->GetPartition().GetValue());

		padtCellDetail->SetLinkId(padtCell->GetIdentityValue());

		if(padtColumn->GetOriginalReportedAmountIndicator().GetValue() == FfsERSystemAssuranceDefinitionColumn::ORIGINAL)
			padtCellDetail->SetAmount(padtCell->GetOriginalAmount().GetValue());
		else if(padtColumn->GetOriginalReportedAmountIndicator().GetValue() = FfsERSystemAssuranceDefinitionColumn::REPORTED)
			padtCellDetail->SetAmount(padtCell->GetTotalAmount().GetValue());

		if(padtParameterGroup->IsSF133Report())
		{
			FfsFundReference adtFund;
			adtFund.SetCodeAspect(padtCell->GetFund().GetValue());
			adtFund.SetBeginningBudgetFYAspect(padtCell->GetBeginningBudgetFiscalYear().GetValue());
			adtFund.SetEndingBudgetFYAspect(padtCell->GetEndingBudgetFiscalYear().GetValue());
			adtFund.AsIdentity();

			padtCellDetail->SetFundId(adtFund.GetIdentityValue());
			padtCellDetail->SetTreasurySymbolId(adtFund.GetFundObj().GetTreasurySymbolId().GetValue());
		}

		delete padtCell;
	}

	return padtCellDetail;
}

FfsERSystemAssuranceReportCellDetailPtr
FfsERSystemAssuranceProcessor::CreateFactsAbstractReportCellDetail(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, 
																   AmsReaderPtr padtReader, AmsString strLineNumber)
{
	FfsFactsAbstractReportDetailPtr padtDetail = NULL;
	FfsERSystemAssuranceReportCellDetailPtr padtCellDetail = NULL;

	padtDetail = (FfsFactsAbstractReportDetailPtr)(GetFactory().CreateSingleInstanceAb(padtReader));

	if(padtDetail)
	{
		FfsERSystemAssuranceDefinitionColumnPtr padtColumn = padtParameterGroup->GetColumnObj();

		padtCellDetail->SetLineNumber(strLineNumber);
		padtCellDetail->SetColumnNumber(padtParameterGroup->GetColumnNumber());
		padtCellDetail->SetPartition(padtDetail->GetPartition().GetValue());

		if(padtParameterGroup->IsFacts1Report())
			padtCellDetail->SetFactsFundGroup(padtDetail->GetFactsFundGroup().GetValue());

		if(padtParameterGroup->IsFacts1PreliminaryReport())
			padtCellDetail->SetFundId(padtDetail->GetFundId().GetValue());

		if(padtParameterGroup->IsFacts2Report())
			padtCellDetail->SetTreasurySymbolId(padtDetail->GetTreasurySymbolId().GetValue());

        if(strTradingPartnerAttributeNumber == "1")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute1Value());
        else if(strTradingPartnerAttributeNumber == "2")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute2Value());
        else if(strTradingPartnerAttributeNumber == "3")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute3Value());
        else if(strTradingPartnerAttributeNumber == "4")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute4Value());
        else if(strTradingPartnerAttributeNumber == "5")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute5Value());
        else if(strTradingPartnerAttributeNumber == "6")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute6Value());
        else if(strTradingPartnerAttributeNumber == "7")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute7Value());
        else if(strTradingPartnerAttributeNumber == "8")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute8Value());
        else if(strTradingPartnerAttributeNumber == "9")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute9Value());
        else if(strTradingPartnerAttributeNumber == "10")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute10Value());
        else if(strTradingPartnerAttributeNumber == "11")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute11Value());
        else if(strTradingPartnerAttributeNumber == "12")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute12Value());
        else if(strTradingPartnerAttributeNumber == "13")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute13Value());
        else if(strTradingPartnerAttributeNumber == "14")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute14Value());
        else if(strTradingPartnerAttributeNumber == "15")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute15Value());
        else if(strTradingPartnerAttributeNumber == "16")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute16Value());
        else if(strTradingPartnerAttributeNumber == "17")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute17Value());
        else if(strTradingPartnerAttributeNumber == "18")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute18Value());
        else if(strTradingPartnerAttributeNumber == "19")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute19Value());
        else if(strTradingPartnerAttributeNumber == "20")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute20Value());
        else if(strTradingPartnerAttributeNumber == "21")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute21Value());
        else if(strTradingPartnerAttributeNumber == "22")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute22Value());
        else if(strTradingPartnerAttributeNumber == "23")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute23Value());
        else if(strTradingPartnerAttributeNumber == "24")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute24Value());
        else if(strTradingPartnerAttributeNumber == "25")
           padtCellDetail->SetTradingPartner(padtDetail->GetAttribute25Value());

		padtCellDetail->SetLinkId(padtDetail->GetIdentityValue());

		if(padtColumn->GetOriginalReportedAmountIndicator().GetValue() == FfsERSystemAssuranceDefinitionColumn::ORIGINAL)
			padtCellDetail->SetAmount(padtCell->GetOriginalAmount().GetValue());
		else if(padtColumn->GetOriginalReportedAmountIndicator().GetValue() = FfsERSystemAssuranceDefinitionColumn::REPORTED)
			padtCellDetail->SetAmount(padtCell->GetReportedAmount().GetValue());

		delete padtDetail;
	}

	return padtCellDetail;
}

FfsERSystemAssuranceReportCellDetailPtr
FfsERSystemAssuranceProcessor::CreateGLRollupCellDetail(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup, 
														AmsReaderPtr padtReader, AmsString strLineNumber)
{
	FfsGLAcctBalancePtr padtBalance = (FfsGLAcctBalancePtr)(GetFactory().CreateSingleInstanceAb(padtReader));
	FfsERSystemAssuranceReportCellDetailPtr padtCellDetail = NULL;

	if(padtBalance)
	{
		padtCellDetail->SetLineNumber(strLineNumber);
		padtCellDetail->SetColumnNumber(padtParameterGroup->GetColumnNumber());
		padtCellDetail->SetPartition(padtBalance->GetPartition().GetValue());
		padtCellDetail->SetFundId(padtBalance->GetFundAsIdentity());
		padtCellDetail->SetTreasurySymbolId(padtBalance->GetTreasurySymbolId().GetValue());

		FfsAgencyReference adtTradingPartner;
		adtTradingPartner.SetCodeAspect(padtBalance->GetTradingPartner().GetValue());
		adtTradingPartner.AsIdentity();

		padtCellDetail->SetTradingPartnerId(adtTradingPartner.GetIdentityAspect());
      
        if(padtCellDetail->GetFundObj())
           padtCellDetail->SetFactsFundGroup(padtCellDetail->GetFundObj()->GetFactsFundGroup());

		padtCellDetail->SetLinkId(padtBalance->GetIdentityValue());
		padtCellDetail->SetAmount(padtBalance->GetDebitBalance().GetValue() - padtBalance->GetCreditBalance().GetValue());

		delete padtBalance;
	}

	return padtCellDetail;
}

AmsVoid
FfsERSystemAssuranceProcessor::CreateTotalsLine(FfsERSystemAssuranceReportPtr padtReport, FfsERSystemAssuranceDefinitionLinePtr padtLine)
{
   	// This method creates the totals lines.  calculateAmounts will actually do the calculations.
	
	for(AmsInt i = 0; i < padtLine->LineTotalCount(); i++)
			{
		FfsERSystemAssuranceDefinitionLineTotalPtr padtTotal = padtLine->GetLineTotal(i);
				AmsString strFromLine = padtTotal->GetLineNumber().GetValue();
				AmsInt iAddOrSubtract = padtTotal->DetermineAddOrSubtract();

				FfsERSystemAssuranceReportLinePtr padtReportLine = CreateNewReportLine(padtReport, padtLine);
			}
		}

AmsVoid
FfsERSystemAssuranceProcessor::CreateTotalsColumn(FfsERSystemAssuranceDefinitionColumnPtr padtColumn,
                                                  FfsERSystemAssuranceReportLinePtr padtReportLine, FfsERSystemAssuranceReportCellDetailPtr padtCell)
		{
	for(AmsInt i = 0; i < padtColumn->ColumnTotalCount(); i++)
			{
		FfsERSystemAssuranceDefinitionColumnTotalPtr padtTotal = padtColumn->GetColumnTotal(i);
				AmsString strFromColumn = padtTotal->GetColumnNumber().GetValue();
				AmsInt iAddOrSubtract = padtTotal->DetermineAddOrSubtract();

		FfsERSystemAssuranceReportLineDetailPtr padtReportLineDetail = CreateNewReportLineDetail(padtReportLine, padtCell);
	}
}

AmsVoid
FfsERSystemAssuranceProcessor::DetermineGLFactory(FfsERSystemAssuranceParameterGroupPtr padtParameterGroup,
												  FfsERSystemAssuranceDefinitionColumnPtr padtColumn, FfsERSystemAssuranceDefinitionCellPtr padtCell)
{
	AmsManyRelationPtr padtColumnDimensions = padtColumn->GetColumnDimensionStrip();
	AmsManyRelationPtr padtCellDimensions = padtCell->GetDimensionStrip();

	if(padtParameterGroup->GetFiscalMonth().isNull() && padtParameterGroup->GetFiscalQuarter().isNull) // annual
	{
        if(ShouldDistributionTableBeUsed(padtColumnDimensions, padtCellDimensions))
		{
			padtParameterGroup->SetFactory(GetPOFactory(FfsGLAcctAnnualBalByDist));
           padtParameterGroup->SetDetalFactory(GetPOFactory(FfsGLAcctAnnualBalByDist));
		}
		else
		{
			padtParameterGroup->SetFactory(GetPOFactory(FfsGLAcctAnnualBalByFund));
           padtParameterGroup->SetDetalFactory(GetPOFactory(FfsGLAcctAnnualBalByFund));
	}
	else
	{
       if(ShouldDistributionTableBeUsed(padtColumnDimensions, padtCellDimensions))
		{
			padtParameterGroup->SetFactory(GetPOFactory(FfsGLAcctPeriodicBalByDist));
          padtParameterGroup->SetDetalFactory(GetPOFactory(FfsGLAcctPeriodicBalByDist));
		}
		else
		{
          padtParameterGroup->SetFactory(GetPOFactory(FfsGLAcctPeriodicBalByFund));
          padtParameterGroup->SetDetalFactory(GetPOFactory(FfsGLAcctPeriodicBalByFund));
    }
}

AmsBoolean
FfsERSystemAssuranceProcessor::ShouldDistributionTableBeUsed(AmsManyRelationPtr padtColumnDimensions, 
                                                             AmsManyRelationPtr padtCellDimensions)
{
   // This method iterates through the column and cell dimension strips to see if anything other than fund
   // dimensions have been entered.  If so, it returns TRUE indicating that a GL Rollup Distribution table
   // should be used for the GL Factory.
   for(AmsInt i = 0; i < padtColumnDimensions->Size(); i++)
   {
      FfsERSystemAssuranceDefinitionColumnDimensionStripPtr padtColumnStrip = padtColumn->GetColumnDimensionStrip(i);
      FfsDimensionStripWithRollups& adtRollupStrip = padtColumnStrip->GetDimensionStrip();

      deque < FfsDimensionReferencePtr > & adtDimensionStripDeque = adtRollupStrip.GetCompleteReferences();
      deque < FfsDimensionReferencePtr > ::iterator adtDimensionStripIterator = adtDimensionStripDeque.begin();
      FfsPartitionReference adtPartitionRef;
      FfsFundReference adtFundRef;

      for( ; adtDimensionStripIterator != adtDimensionStripDeque.end(); adtDimensionStripIterator++)
      {
         FfsDimensionReferencePtr padtDimension = (*adtDimensionStripIterator);

         if((padtDimension->GetLabel() != adtPartitionRef.GetLabel() || padtDimension->GetLabel() != adtFundRef.GetLabel()) &&
             padtDimension->GetCodeAspect().GetValue())
         {
            return TRUE;
		}
	}
}

   for(AmsInt i = 0; i < padtCellDimensions->Size; i++)
   {
      FfsERSystemAssuranceDefinitionCellDimensionStripPtr padtCellStrip = 
         (FfsERSystemAssuranceDefinitionCellDimensionStripPtr)padtCell->GetDimensionStrip(i);
      FfsDimensionStripWithRollups& adtRollupStrip = padtCellStrip->GetDimensionStrip();

      deque < FfsDimensionReferencePtr > & adtDimensionStripDeque = adtRollupStrip.GetCompleteReferences();
      deque < FfsDimensionReferencePtr > ::iterator adtDimensionStripIterator = adtDimensionStripDeque.begin();
      FfsPartitionReference adtPartitionRef;
      FfsFundReference adtFundRef;

      for( ; adtDimensionStripIterator != adtDimensionStripDeque.end(); adtDimensionStripIterator++)
      {
         FfsDimensionReferencePtr padtDimension = (*adtDimensionStripIterator);

         if((padtDimension->GetLabel() != adtPartitionRef.GetLabel() || padtDimension->GetLabel() != adtFundRef.GetLabel()) &&
             padtDimension->GetCodeAspect().GetValue())
         {
            return TRUE;
         }
      }
   }

   return FALSE;
}

AmsString
FfsERSystemAssuranceProcessor::ConvertToPartitionCode(strPartitionId)
{
	FfsPartitionReference adtPartition;
	adtPartition.SetIdentityAspect(strPartitionId);
	adtPartition.AsIdentity();

	return adtPartition.GetPartitionObj().GetCode().GetValue();
}


AmsVoid
FfsERSystemAssuranceProcessor::AddToCriterionIfNotNull(AmsDBCriterion& adtCriterion, AmsTableMapPtr padtTable, 
                        const AmsString& strColumnName, const AmsSting& strValue, const AmsBoolean& bInclude)
{
	if(!strValue.isNull())
		AddToCriterion(adtCriterion, padtTable->GetTable()[strColumnName], strValue), bInclude);
}

AmsVoid
FfsERSystemAssuranceProcessor::AddDimensionStripCriterion(FfsExternalReportAbstractDefinitionDimensionStripPtr padtDimStrip,
														  deque<AmsCriterion> &adtDeque, const AmsBoolean& bInclude, AmsTableMapPtr padtTable)
{
	AmsDBCriterion adtCriterion;

	AddToCriterionIfNotNull(adtCriterion, padtTable, "TSYM", padtDimStrip->GetTreasurySymbol().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "PATN", padtDimStrip->GetDimensionStrip().GetPartition().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "BBFY", padtDimStrip->GetDimensionStrip().GetBegBudgetFY().GetValue(), bInclude);

    AmsDBColumn adtEBFYColumn = padtTable->GetTable()["EBFY"];

    if(padtDimStrip->GetDimensionStrip().GetEndBudgetFY().GetValue().isNull())
    {
        if(bInclude)
           adtCriterion = adtCriterion && adtEBFYColumn.isNull();
        else
           adtCriterion = adtCriterion || !adtEBFYColumn.isNull();
    }

	AddToCriterionIfNotNull(adtCriterion, padtTable, "EBFY", padtDimStrip->GetDimensionStrip().GetEndBudgetFY().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "FUND", padtDimStrip->GetDimensionStrip().GetFund().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "DIV", padtDimStrip->GetDimensionStrip().GetDivision().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "ORGN", padtDimStrip->GetDimensionStrip().GetOrganization().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "SUB_ORGN", padtDimStrip->GetDimensionStrip().GetSubOrganization().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "PROG", padtDimStrip->GetDimensionStrip().GetProgram().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "PROJ", padtDimStrip->GetDimensionStrip().GetProject().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "SUB_PROJ", padtDimStrip->GetDimensionStrip().GetSubProject().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "ACTY", padtDimStrip->GetDimensionStrip().GetActivity().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "BDOB", padtDimStrip->GetDimensionStrip().GetBudgetObject().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "SBOB", padtDimStrip->GetDimensionStrip().GetSubBudgetObject().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "REV_SRCE", padtDimStrip->GetDimensionStrip().GetRevenueSource().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "SREV_SRCE", padtDimStrip->GetDimensionStrip().GetSubRevenueSource().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM1", padtDimStrip->GetDimensionStrip().GetUserDimension1().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM2", padtDimStrip->GetDimensionStrip().GetUserDimension2().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM3", padtDimStrip->GetDimensionStrip().GetUserDimension3().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM4", padtDimStrip->GetDimensionStrip().GetUserDimension4().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM5", padtDimStrip->GetDimensionStrip().GetUserDimension5().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM6", padtDimStrip->GetDimensionStrip().GetUserDimension6().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM7", padtDimStrip->GetDimensionStrip().GetUserDimension7().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM8", padtDimStrip->GetDimensionStrip().GetUserDimension8().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM9", padtDimStrip->GetDimensionStrip().GetUserDimension9().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "USER_DM10", padtDimStrip->GetDimensionStrip().GetUserDimension10().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "REIM_BDOB", padtDimStrip->GetDimensionStrip().GetReimbBudgetObject().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "REIM_SBOB", padtDimStrip->GetDimensionStrip().GetReimbSubBudgetObject().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "CAND_BBFY", padtDimStrip->GetDimensionStrip().GetClosedBegBudgetFY().GetValue(), bInclude);

    AmsDBColumn adtClosedEBFYColumn = padtTable->GetTable()["CAND_EBFY"];

    if(padtDimStrip->GetDimensionStrip().GetClosedEndBudgetFY().GetValue().isNull())
    {
       if(bInclude)
          adtCriterion = adtCriterion && adtClosedEBFYColumn.isNull();
       else
          adtCriterion = adtCriterion || !adtClosedEBFYColumn.isNull();
    }

	AddToCriterionIfNotNull(adtCriterion, padtTable, "CAND_EBFY", padtDimStrip->GetDimensionStrip().GetClosedEndBudgetFY().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, "CAND_FUND", padtDimStrip->GetDimensionStrip().GetClosedFund().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "COST_ORGN", padtDimStrip->GetDimensionStrip().GetCostOrganization().GetValue(), bInclude);
	AddToCriterionIfNotNull(adtCriterion, padtTable, "SCST_ORGN", padtDimStrip->GetDimensionStrip().GetSubCostOrganization().GetValue(), bInclude);

	adtDeque.push_back(adtCriterion);
}

AmsVoid
FfsERSystemAssuranceProcessor::HandleDisplayDiscrepancies(FfsERSystemAssuranceReportPtr padtNewReport)
{
	if(!mbDisplayDiscrepanciesOnlyFlag)
		return;

	for(AmsInt i = 0; i < madtERSystemAssuranceDefinition->ColumnCount(); i++)
	{
		FfsERSystemAssuranceDefinitionColumnPtr padtColumn = (FfsERSystemAssuranceDefinitionColumnPtr) madtERSystemAssuranceDefinition->GetColumn(i);

		if(padtColumn->GetAmountsLiteralsIndicator().GetValue() == FfsExternalReportAbstractDefinitionColumn::TOTAL)
			{
				
		}
	}
}