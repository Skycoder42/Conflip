<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
    <name>PathSyncHelper</name>
    <message>
        <location filename="pathsynchelper.cpp" line="+36"/>
        <source>&amp;Excluded Paths</source>
        <translation>&amp;Ausgeschlossene Pfade</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&lt;p&gt;You can enter regular expressions that are matched against the found paths to exclude them from synchronization. The regex must be a standard PCRE-expression and can contain unicode characters&lt;/p&gt;&lt;p&gt;For example, assuming your path pattern is &quot;dir/file_*.txt&quot; and your directory contains the files&lt;ul&gt;	&lt;li&gt;file_tree.txt&lt;/li&gt;	&lt;li&gt;file_house.txt&lt;/li&gt;	&lt;li&gt;file_heart.txt&lt;/li&gt;&lt;/ul&gt;And you add the exclude pattern &quot;_h\w+e.*\.&quot;, it will only match &quot;file_tree.txt&quot; and exclude the other two&lt;/p&gt;</source>
        <translation>&lt;p&gt;Sie können hier reguläre Ausdrücke eingeben, welche gegen die gefundenen Pfade eingesetzt werden, um die passenden von der Synchronisation auszuschließen. Der Ausdruck muss ein PCRE-Ausdruck sein und darf Unicode enthalten.&lt;/p&gt;
&lt;p&gt;Wenn z.B. der Pfad als &quot;dir/file_*.txt&quot; gegeben ist und die Dateien in dem Ordner sind
&lt;ul&gt;
	&lt;li&gt;file_tree.txt&lt;/li&gt;
	&lt;li&gt;file_house.txt&lt;/li&gt;
	&lt;li&gt;file_heart.txt&lt;/li&gt;
&lt;/ul&gt;
Wenn dann als Ausdruck &quot;_h\w+e.*\.&quot; verwendet wird, dann wird nur file_tree.txt synchronisiert, weil die anderen Beiden vom Ausdruck gematched werden.&lt;/p&gt;</translation>
    </message>
</context>
</TS>
