Work in progress tools for local file backups. Planned features are manual/automatic backups specified from a file, file diff checking, and support for remote drives if possible.

Implementation details:
    * Wildcards cannot be used in the path specified with "in" command (the write location must be singular).
    * A globstar in a path uses the "**" representation and must be separated from other fields with directory separators. For example use "files/**" instead of "files**".
    * If the last entry in a path contains wildcards, it will not match any children paths. Append a globstar if this is desired instead.
