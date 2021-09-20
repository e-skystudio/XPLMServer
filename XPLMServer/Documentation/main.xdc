<?xml version="1.0"?><doc>
<members>
<member name="T:Dataref" decl="false" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="7">
<summary>
This class represent an X-Plane dataref in OOP format.
</summary>
</member>
<member name="M:Dataref.#ctor" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="22">
<summary>
Default empty consturctor
</summary>
</member>
<member name="M:Dataref.#ctor(Dataref!System.Runtime.CompilerServices.IsConst*!System.Runtime.CompilerServices.IsImplicitlyDereferenced)" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="26">
<summary>
Copy Consturctor
</summary>
<param name="rhs">The dataref to be copied.</param>
</member>
<member name="M:Dataref.Dispose" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="31">
<summary>
Destructor
</summary>
</member>
<member name="M:Dataref.Load(std.basic_string&lt;System.SByte!System.Runtime.CompilerServices.IsSignUnspecifiedByte,std.char_traits{System.SByte!System.Runtime.CompilerServices.IsSignUnspecifiedByte},std.allocator&lt;System.SByte!System.Runtime.CompilerServices.IsSignUnspecifiedByte&gt;&gt;)" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="35">
<summary>
Load a dataref from the path.
</summary>


<returns>True if sucessfully loaded</returns>
</member>
<member name="M:Dataref.CanWrite" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="42">
<summary>
Check if the dataref can be written to:
Dataref is not null and XPLMCanWriteDataRef() return true.]]&gt;
</summary>
<returns>True if dataref is not readonly</returns>
</member>
<member name="M:Dataref.IsGood" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="48">
<summary>
Check if the dataref is accessible:
Dataref is not null and XPLMIsDataRefGood() return true.
</summary>
<returns>True if dataref is valid</returns>
</member>
<member name="M:Dataref.GetType" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="54">
<summary>
Return the type of the dataref stored in memory
</summary>
<returns>Dataref::Type</returns>
</member>
<member name="M:Dataref.LoadType" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="59">
<summary>
Ask X-Plane SDK to return the type of the dataref
</summary>
<returns>The dataref type according the SDK</returns>
</member>
<member name="M:Dataref.SetType(Dataref.Type)" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="64">
<summary>
Override the type of the dataref.
</summary>
<param name="newType">The new type of the dataref</param>
</member>
<member name="M:Dataref.GetValue" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="69">
<summary>
Return the current value of the dataref (JSON formated).
</summary>
<returns>The value of the dataref as JSON</returns>
</member>
<member name="M:Dataref.SetValue(std.basic_string&lt;System.SByte!System.Runtime.CompilerServices.IsSignUnspecifiedByte,std.char_traits{System.SByte!System.Runtime.CompilerServices.IsSignUnspecifiedByte},std.allocator&lt;System.SByte!System.Runtime.CompilerServices.IsSignUnspecifiedByte&gt;&gt;)" decl="true" source="C:\Users\rdess\Desktop\XPLMServer\XPLMServer\src\Dataref.h" line="74">
<summary>
Send a new value to the dataref.
</summary>
<param name="value">The value to be sent to the dataref (as JSON)</param>
</member>
</members>
</doc>