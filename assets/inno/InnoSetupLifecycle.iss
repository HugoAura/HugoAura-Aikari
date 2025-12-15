[Code]
procedure CurStepChanged(CurStep: TSetupStep);
begin
  VCRedist_CurStepChanged(CurStep);
  case CurStep of
    ssInstall:
      G_ReportStatus('INST_FILES');
    ssPostInstall:
    begin
      if G_PostInst_NeedToInstallVCRedist then
      begin
        G_ReportStatus('INST_VC_REDIST');
      end;
    end;
    ssDone:
      G_ReportStatus('DONE');
  end;
end;

procedure InitializeWizard;
begin
  VCRedist_InitializeWizard();
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
    Result := True;

    if (CurPageID = wpReady) and G_PostInst_NeedToInstallVCRedist then
    begin
        Result := VCRedist_HandleNextBtnClick();
    end;
end;
