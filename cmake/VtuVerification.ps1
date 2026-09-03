# Test utilities for the solver's ASCII Float64 result fields, not a general VTK reader.
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
