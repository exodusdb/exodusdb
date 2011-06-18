using System.Reflection;
using System.Runtime.CompilerServices;

//
// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// that is associated with an assembly.
//
[assembly: AssemblyTitle("Exodus")]
[assembly: AssemblyDescription("Exodus")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyCompany("Exodus")]
[assembly: AssemblyProduct("Exodus")]
[assembly: AssemblyCopyright("Exodus")]
[assembly: AssemblyTrademark("Exodus")]
[assembly: AssemblyCulture("")]

//
// Version information for an assembly is made up of the following four values:
//
//      Major Version
//      Minor Version 
//      Build Number
//      Revision
//
// You can specify all the values, or you can default the revision and build numbers 
// by using the '*' as shown below:

[assembly: AssemblyVersion("1.0.*")]

//
// To sign your assembly you must specify a key to use. See the 
// Microsoft .NET Framework documentation for more information about assembly signing.
//
// Use the following attributes to control that key is used for signing. 
//
// Notes: 
//   (*) If no key is specified, the assembly is not signed.
//   (*) KeyName refers to a key that has been installed in the Crypto Service
//       Provider (CSP) on your computer. KeyFile refers to a file that contains
//       a key.
//   (*) If the KeyFile and the KeyName values are both specified, the 
//       following processing occurs:
//       (1) If the KeyName can be found in the CSP, that key is used.
//       (2) If the KeyName does not exist and the KeyFile does exist, the key 
//           in the KeyFile is installed to the CSP and used.
//   (*) To create a KeyFile, you can use the sn.exe (Strong Name) utility.
//       When specifying the KeyFile, the location of the KeyFile must be
//       relative to the project output directory which is
//       %Project Directory%\obj\<configuration>. For example, if your KeyFile is
//       located in the project directory, you would specify the AssemblyKeyFile 
//       attribute as [assembly: AssemblyKeyFile("..\\..\\mykey.snk")]
//   (*) Delay Signing is an advanced option - see the Microsoft .NET Framework
//       documentation for more information about this.
//
[assembly: AssemblyDelaySign(false)]
[assembly: AssemblyKeyFile("..\\exodus_library.snk")]
[assembly: AssemblyKeyName("")]
