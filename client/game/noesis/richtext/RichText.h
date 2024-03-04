////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __APP_RICHTEXT_H__
#define __APP_RICHTEXT_H__


#include <NsCore/Noesis.h>
#include <NsCore/ReflectionDeclare.h>


namespace Noesis
{
class DependencyObject;
class DependencyProperty;
}

namespace NoesisApp
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Adds a *Text* attached property for TextBlock which formats
/// `BBCode <https://www.bbcode.org/reference.php>`_ into Inlines.
///
/// .. code-block:: xml
///
///    <Grid
///      xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
///      xmlns:noesis="clr-namespace:NoesisGUIExtensions;assembly=Noesis.GUI.Extensions">
///      <TextBlock noesis:RichText.Text="Plain. [b]Bold, [i]bold italic, [/i]
///        [size=60]Size 60 text.[/size] [color=Red]Red text.[/color] [img height=80]disk.png[/img]
///        [br/] [url='https://www.noesisengine.com/']NoesisEngine.com[/url]" />
///    </Grid>
///
/// .. admonition:: NOTE
///
///   In Unreal, this class is implemented using Unreal-style markup syntax. More information is
///   available in this `Localization Tutorial <Gui.Core.LocalizationTutorial.html#unreal-engine>`_
///
/// Default supported BBCode tags, with their Inline output:
///
/// * *b*: Bold,
///   `"[b]bold.[/b]"`
///
/// * *i*: Italic,
///   `"[i]italic.[/i]"`
///
/// * *u*: Underline,
///   `"[u]underline.[/u]"`
///
/// * *size*: Span with FontSize set to the parameter value,
///   `"[size=60]size 60 text.[/size]"`
///
/// * *font*: Span with FontFamily set to the parameter value,
///   `"[font='#PT Root UI']PT Root UI font.[/]"`
///
/// * *color*: Span with Foreground set to the parameter value (a color name, or ARBG hex),
///   `"[color=Red]red.[/color][color=#FF0000FF]blue.[/color]"`
///
/// * *url*: Hyperlink with NavigateUri set to the parameter value,
///   `"[url='https://www.noesisengine.com/']NoesisEngine.com[/url]"`
///
/// * *br*: A LineBreak,
///   `"Line one.[br/]Line two."`
///
/// * *img*: Image contained in an InlineUIContainer,
///   `"[img height=80]disk.png[/img]"`
///
/// * *style*: Span with the Style property set to the resource key provided by the parameter value,
///   `"[style='Header1']Styled text.[/style]"`
///
///  .. code-block:: xml
///
///      <Grid
///        xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
///        xmlns:noesis="clr-namespace:NoesisGUIExtensions;assembly=Noesis.GUI.Extensions">
///        <Grid.Resources>
///          <Style x:Key="Header1" TargetType="{x:Type Span}">
///            <Setter Property="FontSize" Value="30"/>
///            <Setter Property="Foreground" Value="Green"/>
///          </Style>
///        </Grid.Resources>
///        <TextBlock noesis:RichText.Text="[style='Header1']Styled text.[/style]" />
///      </Grid>
///
/// * *bind*: Run containing a Binding with the Path property set to the tag contents. This tag
///   has an optional "format" parameter which can be used to modify the StringFormat property
///   of the Binding,
///   `"[bind format='{0:0}']Path[/bind]"`
///
///  .. code-block:: xml
///
///      <Grid
///        xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
///        xmlns:noesis="clr-namespace:NoesisGUIExtensions;assembly=Noesis.GUI.Extensions"
///        xmlns:local="clr-namespace:MyGame">
///        <Grid.DataContext>
///          <local:MyViewModel CurrentHealth="66.75" MaxHealth="100" />
///        </Grid.DataContext>
///        <TextBlock noesis:RichText.Text="Health is [bind format='{0:0}']CurrentHealth[/bind] out of
///          [bind format='{0:0}']MaxHealth[/bind]" />
///      </Grid>
///
////////////////////////////////////////////////////////////////////////////////////////////////////
struct RichText
{
    /// Gets the value of an element's RichText Text property
    static const char* GetText(const Noesis::DependencyObject* element);
    /// Sets the value of an element's RichText Text property
    static void SetText(Noesis::DependencyObject* element, const char* value);

    static const Noesis::DependencyProperty* TextProperty;

    NS_DECLARE_REFLECTION(RichText, Noesis::NoParent)
};

}


#endif
