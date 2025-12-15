[Code]
procedure RunCommand(Cmd: String; Params: String);
var
  ResultCode: Integer;
begin
  Exec(Cmd, Params, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  RunCommand('sc.exe', 'stop HugoAuraAikari');
  Sleep(3000);
  RunCommand('taskkill.exe', '/F /IM Aikari-Launcher.exe');

  Result := '';
end;
