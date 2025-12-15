[Code]
var
  G_ProgressFilePath: String;
  G_PostInst_NeedToInstallVCRedist: Boolean;

procedure G_ReportStatus(Status: String);
begin
  if G_ProgressFilePath = '' then
    G_ProgressFilePath := ExpandConstant('{param:PROGRESS_FILE}');
  
  if G_ProgressFilePath <> '' then
    SaveStringToFile(G_ProgressFilePath, Status + #13#10, False);
end;
