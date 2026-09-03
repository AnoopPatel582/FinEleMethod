# Test utilities for the solver's ASCII Float64 result fields, not a general VTK reader.
function Assert-NearValues([double[]]$Actual, [double[]]$Expected, [string]$Label) {
    if ($Actual.Count -ne $Expected.Count) { throw "$Label value count differs from analytical expectation." }
    for ($index = 0; $index -lt $Actual.Count; $index++) {
        if ([double]::IsNaN($Actual[$index]) -or [double]::IsInfinity($Actual[$index]) -or
            [Math]::Abs($Actual[$index] - $Expected[$index]) -gt
            1e-8 * [Math]::Max(1.0, [Math]::Abs($Expected[$index]))) {
            throw "$Label component $index differs: actual=$($Actual[$index]), expected=$($Expected[$index])."
        }
    }
}

# Expectations apply only to the three shipped unit-square/unit-cube examples.
function Assert-ExampleSolution([System.Xml.XmlElement]$Piece, [string]$Model) {
    switch ($Model) {
        'q4_tension' {
            $displacement = @(0,0,0, .01,0,0, .01,-.0025,0, 0,-.0025,0)
            $strain = @(.01,-.0025,0); $stress = @(10,0,0)
            $principal = @(10,0,0); $vonMises = 10.0; $reaction = @(-10,0,0)
        }
        'q4_plane_strain_tension' {
            $displacement = @(0,0,0, .009375,0,0, .009375,-.003125,0, 0,-.003125,0)
            $strain = @(.009375,-.003125,0,0); $stress = @(10,0,2.5,0)
            $principal = @(10,2.5,0); $vonMises = [Math]::Sqrt(81.25); $reaction = @(-10,0,0)
        }
        'h8_compression' {
            $displacement = @(0,0,0, .0025,0,0, .0025,.0025,0, 0,.0025,0,
                0,0,-.01, .0025,0,-.01, .0025,.0025,-.01, 0,.0025,-.01)
            $strain = @(.0025,.0025,-.01,0,0,0); $stress = @(0,0,-10,0,0,0)
            $principal = @(0,0,-10); $vonMises = 10.0; $reaction = @(0,0,10)
        }
        default { throw "No analytical package expectation for model: $Model" }
    }
    $nodes = [int]$Piece.NumberOfPoints
    Assert-NearValues (Read-VtuField $Piece PointData Displacement $nodes 3) $displacement "$Model displacement"
    Assert-NearValues (Read-VtuField $Piece CellData Strain 1 $strain.Count) $strain "$Model strain"
    Assert-NearValues (Read-VtuField $Piece CellData Stress 1 $stress.Count) $stress "$Model stress"
    Assert-NearValues (Read-VtuField $Piece CellData PrincipalStress 1 3) $principal "$Model principal stress"
    Assert-NearValues (Read-VtuField $Piece CellData VonMises 1 1) @($vonMises) "$Model von Mises"
    $reactions = Read-VtuField $Piece PointData ReactionForce $nodes 3
    $totals = [double[]]@(0,0,0)
    for ($index = 0; $index -lt $reactions.Count; $index++) { $totals[$index % 3] += $reactions[$index] }
    Assert-NearValues $totals $reaction "$Model total reaction"
}

function Read-VtuField {
    param(
        [System.Xml.XmlElement]$Piece,
        [ValidateSet('PointData', 'CellData')][string]$Association,
        [string]$Name,
        [int]$Tuples,
        [int]$Components
    )
    $arrays = $Piece.SelectNodes("$Association/DataArray[@Name='$Name']")
    if ($arrays.Count -ne 1) { throw "VTU field $Name must occur exactly once." }
    $array = $arrays[0]
    $actualComponents = 1
    if ($array.HasAttribute('NumberOfComponents')) {
        $actualComponents = [int]$array.GetAttribute('NumberOfComponents')
    }
    if ($actualComponents -ne $Components -or $array.GetAttribute('format') -ne 'ascii' -or
        $array.GetAttribute('type') -ne 'Float64') {
        throw "VTU field $Name has unexpected components or encoding."
    }
    $tokens = @($array.InnerText.Trim() -split '\s+' | Where-Object { $_ -ne '' })
    if ($tokens.Count -ne $Tuples * $Components) { throw "VTU field $Name has incorrect value count." }
    $values = [double[]]::new($tokens.Count)
    for ($index = 0; $index -lt $tokens.Count; $index++) {
        $value = 0.0
        if (-not [double]::TryParse($tokens[$index], [System.Globalization.NumberStyles]::Float,
            [System.Globalization.CultureInfo]::InvariantCulture, [ref]$value) -or
            [double]::IsNaN($value) -or [double]::IsInfinity($value)) {
            throw "VTU field $Name contains a non-finite or invalid number."
        }
        $values[$index] = $value
    }
    return ,$values
}
