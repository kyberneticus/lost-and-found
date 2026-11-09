# Lost and Found Management System

## Windows prerequisites

Open **PowerShell as Administrator** in the project root, then run:

```powershell
.\scripts\install-windows-prerequisites.ps1
```

The script checks `winget`, refreshes its source, and installs the Visual C++ Build Tools and MariaDB (which supplies `<mysql/mysql.h>` and its client library). It stops with the official App Installer prerequisite if `winget` is unavailable. Open a new terminal after it finishes; configure `DB_HOST`, `DB_USER`, `DB_PASSWORD`, and `DB_NAME` before running the application.
