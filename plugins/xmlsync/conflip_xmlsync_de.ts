<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
    <name>XmlSyncHelper</name>
    <message>
        <location filename="xmlsynchelper.cpp" line="+28"/>
        <source>&amp;Keys</source>
        <translation>&amp;Schlüssel</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&lt;p&gt;Enter the paths of the XML-Elements you want to synchronize. Elements are seperated by a &apos;/&apos; and there are some special characters that match special things. The possible variants are:&lt;ul&gt;	&lt;li&gt;&quot;some/node&quot;: A path of elements. This will synchronize the element &quot;node&quot; within the		element &quot;some&quot; inside the root element. All attributes, child-elements and text are synchronized&lt;/li&gt;	&lt;li&gt;&quot;node/~&quot;: This will synchronize the text within the &quot;node&quot; element, and nothing else&lt;/li&gt;	&lt;li&gt;&quot;node/=&quot;: This will synchronize all attributes of &quot;node&quot; but not the contents&lt;/li&gt;	&lt;li&gt;&quot;node/=key&quot;: This will synchronize the attribute &quot;key&quot; of &quot;node&quot; but not the contents&lt;/li&gt;	&lt;li&gt;&quot;node/#&quot;: This will synchronize all attributes and content text of &quot;node&quot;&lt;/li&gt;	&lt;li&gt;&quot;node/&quot;: This will synchronize all child elements in &quot;node&quot;, but no attributes&lt;/li&gt;&lt;/ul&gt;&lt;/p&gt;&lt;p&gt;You never have to specify the root element, as a XML-Document can only have a single root.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Geben Sie den Pfad zu den XML-Elementen ein, die synchronisiert werden sollen. Elemente werden durch ein &apos;/&apos; getrennt und es gibt einige Spezial-Symbole für Dinge innerhalb von Elementen. Die verschiedenen Varianten sind:
&lt;ul&gt;
	&lt;li&gt;&quot;some/node&quot;: Ein Pfad von Elementen. Hier würde das Element &quot;node&quot; innerhalb des Elements &quot;some&quot; innerhalb des Wurzel-Elements synchronisiert. Alle Attribute, Unterelemente und Text werden mit synchronisiert&lt;/li&gt;
	&lt;li&gt;&quot;node/~&quot;: Synchronisiert den Text des &quot;node&quot; Elements, aber sonst nichts&lt;/li&gt;
	&lt;li&gt;&quot;node/=&quot;: Synchronisiert alle Attribute des &quot;node&quot; Elements, aber keine &quot;Inhalte&quot; des Elements&lt;/li&gt;
	&lt;li&gt;&quot;node/=key&quot;: Synchronisiert nur das Attribut &quot;key&quot; des &quot;node&quot; Elements, und sonst nichts&lt;/li&gt;
	&lt;li&gt;&quot;node/#&quot;: Synchronisiert alle Attribute und den Text des &quot;node&quot; Elements, aber keine Unterelemente&lt;/li&gt;
	&lt;li&gt;&quot;node/&quot;: Synchronisiert alle Unterelemente des &quot;Node&quot; Elements, aber keiner seiner Attribute&lt;/li&gt;
&lt;/ul&gt;&lt;/p&gt;
&lt;p&gt;Das Wurzel-Element muss dabei nie explizit Angegeben werden, da XML-Dokumente nur ein Wurzel-Element haben können.&lt;/p&gt;</translation>
    </message>
</context>
</TS>
