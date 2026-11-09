[CmdletBinding(SupportsShouldProcess = $true)]
param()

$ErrorActionPreference = 'Stop'

if (-not (Get-Command winget.exe -ErrorAction SilentlyContinue)) {
    throw 'winget was not found. Install Microsoft App Installer, then run this script again.'
}

function Install-WingetPackage {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Id,
        [string[]]$ExtraArguments = @()
    )

    if ($PSCmdlet.ShouldProcess($Id, 'Install with winget')) {
        & winget.exe install --id $Id --exact --silent --disable-interactivity `
            --accept-package-agreements --accept-source-agreements @ExtraArguments
        if ($LASTEXITCODE -ne 0) {
            throw "winget could not install $Id (exit code $LASTEXITCODE)."
        }
    }
}

Write-Host "Using winget $(& winget.exe --version)"
& winget.exe source update
if ($LASTEXITCODE -ne 0) {
    throw "winget source update failed (exit code $LASTEXITCODE)."
}

# MariaDB Server's Windows installer provides the C client headers and libraries
# required by src/main.cpp (<mysql/mysql.h>).
Install-WingetPackage -Id 'Microsoft.VisualStudio.2022.BuildTools' -ExtraArguments @(
    '--override', '--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'
)
Install-WingetPackage -Id 'MariaDB.Server'

Write-Host 'Windows prerequisites are installed. Open a new terminal before building.'
