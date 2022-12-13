/*
  ==============================================================================

    FaderSlider.cpp
    Created: 21 Nov 2022 9:26:21am
    Author:  bgill

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FaderSlider.h"

//==============================================================================
FaderSliderLookAndFeel::FaderSliderLookAndFeel() : LookAndFeel_V4()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    //thumbImage = ImageCache::getFromMemory(BinaryData::outputonlinepngtools_png, BinaryData::outputonlinepngtools_pngSize);
    thumbImage = ImageCache::getFromMemory(FaderBinaryData::faderalphanonflat_png, FaderBinaryData::faderalphanonflat_pngSize);
}

FaderSliderLookAndFeel::~FaderSliderLookAndFeel()
{
}

void FaderSliderLookAndFeel::drawLinearSliderBackground(Graphics& g, int x, int y, int width, int height,
    float, float, float,
    const Slider::SliderStyle, Slider& slider)
{
    //g.setColour(Colours::red);
    //g.drawRect(x, y, width, height, 1.0);
    const float sliderRadius = (float)getSliderThumbRadius(slider) * 0.5f; // smaller track than v2

    const Colour trackColour(slider.findColour(Slider::trackColourId));
    const Colour gradCol1(trackColour.overlaidWith(Colour(slider.isEnabled() ? 0x13000000 : 0x09000000)));
    const Colour gradCol2(trackColour.overlaidWith(Colour(0x06000000)));
    Path indent;

    if (slider.isHorizontal())
    {
        auto iy = (float)y + (float)height * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill(ColourGradient::vertical(gradCol1, iy, gradCol2, iy + sliderRadius));

        indent.addRoundedRectangle((float)x - sliderRadius * 0.5f, iy, (float)width + sliderRadius, sliderRadius, 5.0f);
    }
    else
    {
        auto ix = (float)x + (float)width * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill(ColourGradient::horizontal(gradCol1, ix, gradCol2, ix + sliderRadius));

        indent.addRoundedRectangle(ix, (float)y - sliderRadius * 0.5f, sliderRadius, (float)height + sliderRadius, 5.0f);
    }

    g.fillPath(indent);

    g.setColour(trackColour.contrasting(0.5f));
    g.strokePath(indent, PathStrokeType(0.5f));
}

void FaderSliderLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    const Slider::SliderStyle style, Slider& slider)
{
    drawLinearSliderBackground(g, x, y, width, height,
        sliderPos, minSliderPos, maxSliderPos,
        style, slider);

    if (thumbImage.isValid())
    {
        int imgWidth = thumbImage.getWidth();
        int imgHeight = thumbImage.getHeight();

        const float centerX = width * 0.5f;

        g.drawImage(thumbImage, (int)centerX - (imgWidth / 2), (int)sliderPos - (imgHeight / 2), imgWidth, imgHeight, 0, 0, imgWidth, imgHeight);
    }
    else
    {
        static const float textPpercent = 0.35f;
        Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f, 0.5f * height, width * textPpercent, 0.5f * height);

        g.setColour(Colours::white);

        g.drawFittedText(String("No Image"), text_bounds.getSmallestIntegerContainer(), Justification::horizontallyCentred | Justification::centred, 1);
    }
}
int FaderSliderLookAndFeel::getSliderThumbRadius(Slider&)
{
    return 21;
}
void FaderSliderLookAndFeel::drawLabel(Graphics& g, Label& label)
{
    g.setColour(Colour(uint8(0), 0, 0, 1.0f));
    auto area = label.getLocalBounds();
    g.fillRoundedRectangle(label.getLocalBounds().removeFromBottom(18).toFloat(), 3.0f);


    if (!label.isBeingEdited())
    {
        const Font font(getLabelFont(label));

        g.setColour(Colour(uint8(255), 255, 255, 1.0f));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds()).removeFromBottom(15);

        auto textValue = label.getTextValue();
        auto text = String((double)textValue.getValue(), 2);
        g.drawFittedText(text, textArea, label.getJustificationType(),
            1,
            label.getMinimumHorizontalScale());

        g.setColour(Colour(uint8(255), 255, 255, 0.1f));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(Label::outlineColourId));
    }

    //g.fillRoundedRectangle(label.getLocalBounds().toFloat(), 3.0f);
}
//==============================================================================
FaderSlider::FaderSlider() : punch::SmoothSlider(0.8f,0.05f)
{
    setLookAndFeel(&faderSliderLookAndFeel);
}

FaderSlider::~FaderSlider()
{
    setLookAndFeel(nullptr);
}

namespace FaderBinaryData
{
//================== faderalphanonflat.png ==================
static const unsigned char temp_binary_data_3[] =
"\x89PNG\r\n"
"\x1a\n"
"\0\0\0\rIHDR\0\0\0\x14\0\0\0-\x08\x06\0\0\0\xae\xfcKh\0\0\0\tpHYs\0\0\x0e\xc4\0\0\x0e\xc4\x01\x95+\x0e\x1b\0\0""B\x01iTXtXML:com.adobe.xmp\0\0\0\0\0<?xpacket begin=\"\xef\xbb\xbf\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
"<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 5.6-c067 79.157747, 2015/03/30-23:40:42        \">\n"
"   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
"      <rdf:Description rdf:about=\"\"\n"
"            xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\"\n"
"            xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
"            xmlns:photoshop=\"http://ns.adobe.com/photoshop/1.0/\"\n"
"            xmlns:xmpMM=\"http://ns.adobe.com/xap/1.0/mm/\"\n"
"            xmlns:stEvt=\"http://ns.adobe.com/xap/1.0/sType/ResourceEvent#\"\n"
"            xmlns:stRef=\"http://ns.adobe.com/xap/1.0/sType/ResourceRef#\"\n"
"            xmlns:tiff=\"http://ns.adobe.com/tiff/1.0/\"\n"
"            xmlns:exif=\"http://ns.adobe.com/exif/1.0/\">\n"
"         <xmp:CreatorTool>Adobe Photoshop CC 2015 (Windows)</xmp:CreatorTool>\n"
"         <xmp:CreateDate>2022-11-08T11:54:26-06:00</xmp:CreateDate>\n"
"         <xmp:ModifyDate>2022-12-08T06:55:19-06:00</xmp:ModifyDate>\n"
"         <xmp:MetadataDate>2022-12-08T06:55:19-06:00</xmp:MetadataDate>\n"
"         <dc:format>image/png</dc:format>\n"
"         <photoshop:ColorMode>3</photoshop:ColorMode>\n"
"         <xmpMM:InstanceID>xmp.iid:204fcf86-09c2-134a-ba64-778f35156c33</xmpMM:InstanceID>\n"
"         <xmpMM:DocumentID>adobe:docid:photoshop:7742492e-76f7-11ed-b307-a76dc0d0d01e</xmpMM:DocumentID>\n"
"         <xmpMM:OriginalDocumentID>xmp.did:2db6ffb6-ef62-e041-addf-fbc3db31823c</xmpMM:OriginalDocumentID>\n"
"         <xmpMM:History>\n"
"            <rdf:Seq>\n"
"               <rdf:li rdf:parseType=\"Resource\">\n"
"                  <stEvt:action>created</stEvt:action>\n"
"                  <stEvt:instanceID>xmp.iid:2db6ffb6-ef62-e041-addf-fbc3db31823c</stEvt:instanceID>\n"
"                  <stEvt:when>2022-11-08T11:54:26-06:00</stEvt:when>\n"
"                  <stEvt:softwareAgent>Adobe Photoshop CC 2015 (Windows)</stEvt:softwareAgent>\n"
"               </rdf:li>\n"
"               <rdf:li rdf:parseType=\"Resource\">\n"
"                  <stEvt:action>converted</stEvt:action>\n"
"                  <stEvt:parameters>from image/png to application/vnd.adobe.photoshop</stEvt:parameters>\n"
"               </rdf:li>\n"
"               <rdf:li rdf:parseType=\"Resource\">\n"
"                  <stEvt:action>saved</stEvt:action>\n"
"                  <stEvt:instanceID>xmp.iid:b03f6496-2d86-1a40-bb79-afd29c990618</stEvt:instanceID>\n"
"                  <stEvt:when>2022-12-06T14:22:14-06:00</stEvt:when>\n"
"                  <stEvt:softwareAgent>Adobe Photoshop CC 2015 (Windows)</stEvt:softwareAgent>\n"
"                  <stEvt:changed>/</stEvt:changed>\n"
"               </rdf:li>\n"
"               <rdf:li rdf:parseType=\"Resource\">\n"
"                  <stEvt:action>saved</stEvt:action>\n"
"                  <stEvt:instanceID>xmp.iid:a60c33bc-5307-494c-a83a-d88d608a1964</stEvt:instanceID>\n"
"                  <stEvt:when>2022-12-08T06:55:19-06:00</stEvt:when>\n"
"                  <stEvt:softwareAgent>Adobe Photoshop CC 2015 (Windows)</stEvt:softwareAgent>\n"
"                  <stEvt:changed>/</stEvt:changed>\n"
"               </rdf:li>\n"
"               <rdf:li rdf:parseType=\"Resource\">\n"
"                  <stEvt:action>converted</stEvt:action>\n"
"                  <stEvt:parameters>from application/vnd.adobe.photoshop to image/png</stEvt:parameters>\n"
"               </rdf:li>\n"
"               <rdf:li rdf:parseType=\"Resource\">\n"
"                  <stEvt:action>derived</stEvt:action>\n"
"                  <stEvt:parameters>converted from application/vnd.adobe.photoshop to image/png</stEvt:parameters>\n"
"               </rdf:li>\n"
"               <rdf:li rdf:parseType=\"Resource\">\n"
"                  <stEvt:action>saved</stEvt:action>\n"
"                  <stEvt:instanceID>xmp.iid:204fcf86-09c2-134a-ba64-778f35156c33</stEvt:instanceID>\n"
"                  <stEvt:when>2022-12-08T06:55:19-06:00</stEvt:when>\n"
"                  <stEvt:softwareAgent>Adobe Photoshop CC 2015 (Windows)</stEvt:softwareAgent>\n"
"                  <stEvt:changed>/</stEvt:changed>\n"
"               </rdf:li>\n"
"            </rdf:Seq>\n"
"         </xmpMM:History>\n"
"         <xmpMM:DerivedFrom rdf:parseType=\"Resource\">\n"
"            <stRef:instanceID>xmp.iid:a60c33bc-5307-494c-a83a-d88d608a1964</stRef:instanceID>\n"
"            <stRef:documentID>adobe:docid:photoshop:99f7dc21-75a3-11ed-90be-9fe2fbe79816</stRef:documentID>\n"
"            <stRef:originalDocumentID>xmp.did:2db6ffb6-ef62-e041-addf-fbc3db31823c</stRef:originalDocumentID>\n"
"         </xmpMM:DerivedFrom>\n"
"         <tiff:Orientation>1</tiff:Orientation>\n"
"         <tiff:XResolution>960000/10000</tiff:XResolution>\n"
"         <tiff:YResolution>960000/10000</tiff:YResolution>\n"
"         <tiff:ResolutionUnit>2</tiff:ResolutionUnit>\n"
"         <exif:ColorSpace>65535</exif:ColorSpace>\n"
"         <exif:PixelXDimension>20</exif:PixelXDimension>\n"
"         <exif:PixelYDimension>45</exif:PixelYDimension>\n"
"      </rdf:Description>\n"
"   </rdf:RDF>\n"
"</x:xmpmeta>\n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                                                                                                    \n"
"                            \n"
"<?xpacket end=\"w\"?>\x07\xf4\n"
"\xee\0\0\0 cHRM\0\0z%\0\0\x80\x83\0\0\xf9\xff\0\0\x80\xe9\0\0u0\0\0\xea`\0\0:\x98\0\0\x17o\x92_\xc5""F\0\0\x0eMIDATx\x01\0=\x0e\xc2\xf1\x01\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x04\0\0\0\0`jj\xfe\xfa\xfa\xfa\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfa\xfa\xfa\0\x06\x06\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfb\xf7\xf7\0\xce\xc7\xc7\0\xdd\xde\xde\x02\x04"
"\0\0\0\0\x12\x14\x14\0\xfc\xfc\xfc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfc\xfc\xfc\0\x04\x04\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xf1\xec\xec\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\x17\x19\x19\0\xff\xfd\xfd\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfe\xfe\xfe\0\x02\x02\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfe\xfa\xfa\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\xc0\xbc\xbc\0\xfb\xf9\xf9\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfc\xfc\xfc\0\x04\x04\x04\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x05\x01\x01\0\x01\x01\x01\0\0\0\0\0\x04\0\0\0\0\xda\xd6\xd6\0\xf9\xf8\xf8\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfa\xfa\xfa\0\x06\x06\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x15"
"\x13\x13\0\xfe\xff\xff\0\x01\x01\x01M\x04\0\0\0\0\x02\x02\x02\0\x01\x02\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x01\x01\0\x01\x02\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x01\x01\0\0\xff\xff\0\0\0\0\0\x04\0\0\0\0\x02"
"\x01\x01\0\x02\x03\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x03\x03\0\x02\x03\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x01\x01\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\0\x02\x02\0\x01\x01\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\x01\x01\x01\0\x01\x01\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x01\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\x03\x02\x02\0\x03\x02\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x01\x01\0\x03\x02\x02\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x01\x01\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\x02\x03\x03\0\x03\x03\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x03\x03\0\x03\x03\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xff\x01\x01\0\x01\x01\x01"
"\0\0\0\0\0\x04\0\0\0\0\x02\x01\x01\0\x01\x01\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x01\x01\0\x01\x01\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x01\x01\0\xfe\xfe\xfe\0\0\0\0\0\x04\0\0\0\0\x02\x03\x03\0\x03\x03\x03\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\x03\x03\0\x03\x03\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x02\x02\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\x03\x02\x02\0\x02\x02\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x02"
"\x02\0\x02\x02\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x01\x01\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\x02\x03\x03\0\x03\x04\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x03\x03\0\x03\x04\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\x02\x02\x02\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\xef\xee\xee\0\x0f\x11\x11\0\xfe\xfc\xfc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x01\x01\0\x02\x01\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x01\0\x02\x03\x03\0\xf2\xf2\xf2\0\x03\x02\x02\0"
"\0\0\0\0\x04\0\0\0\0\xe4\xe3\xe3\0\x02\x02\x02\0\x14\x15\x15\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x02\0\t\x08\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfb\xfb\xfb\0\xf1\xf0\xf0\0\xf2\xf0\xf0\0\x0b\x07\x07\0\0\0\0\0\x04\0\0\0\0\t\x04\x04\0\xeb"
"\xe5\xe5\0\xeb\xe5\xe5\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xf0\xf0\xf0\0\x10\x10\x10\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x0e\x0e\x0e\0\x0c\x0c\x0c\0\0\0\0\0\x04\0\0\0\0\xf7\xfc\xfc\0""7::\0\x14\x17\x17\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\xfa\xfa\xfa\0\x06\x06\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xec\xe9\xe9\0\xd9\xd5\xd5\0\xf4\xf4\xf4\0\0\0\0\0\x04\0\0\0\0-//\0\x11\x10\x10\0\xfb\xf7\xf7\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\0\0\0\xfb\xf7\xf7\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x11\x10\x10\0\"##\0\xe6\xe1\xe1\0\0\0\0\0\x04\0\0\0\0\r\x0e\x0e\0\xec\xeb\xeb\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xf5\xf6\xf6\0\x0b\n"
"\n"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x01\0\x04\x05\x05\0\x03\x03\x03\0\0\0\0\0\x04\0\0\0\0\x02\x03\x03\0\x03\x04\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x04\x03\x03\0\x03\x04\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\x06\x06\x06\0\xfe\xfe\xfe\0\0\0\0\0\x04\0\0\0\0\x06\x06\x06\0\0\xff\xff\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x05\xfd\xfd\0\x05\x06\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x05\x05\x05\0\0\xff\xff\0\0\0\0\0\x04\0\0\0\0\t"
"\t\t\0\x02\x02\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xff\0\0\0\x08\t\t\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x05\x03\x03\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\t\n"
"\n"
"\0\x02\x03\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\x02\x02\0\x07\x08\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x04\x01\x01\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\x08\n"
"\n"
"\0\x03\x03\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x05\x05\x05\0\x05\x05\x05\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\x01\0\x02\x01\x01\0\0\0\0\0\x04\0\0\0\0\x0b\x0b\x0b\0\xfd\x08\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\xfc\xfc\xfc\0\x04\x04\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfe\xfd\xfd\0\x01\x02\x02\0\0\0\0\0\x04\0\0\0\0\x0e\x0e\x0e\0\xfb\xfc\xfc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfd\xfd\xfd\0\x03\x03\x03\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfe\xfc\xfc\0\xfd\xfe\xfe\0\0\0\0\0\x04\0\0\0\0\r\x10\x10\0\xff\xfe\xfe\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfe\xff\xff\0\x02\x01\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xf9\xf8\xf8\0"
"\0\0\0\0\0\0\0\0\x04\0\0\0\0\xe8\xe5\xe5\0\xfc\xfc\xfc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfc\xfc\xfc\0\x04\x04\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xff\xfd\xfd\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\xc6\xc0\xc0\0\xf9\xf8\xf8\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xf3\xf3\xf3\0\r\r\r\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\t\t\t\0\x02\x01\x01\0\0\0\0\0\x04\0\0\0\0\xfd\xfd\xfd\0\xfe\xfd\xfd\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfd\xfd\xfd\0"
"\xfe\xfd\xfd\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfd\xfc\xfc\0\x02\x03\x03\0\0\0\0\0\x04\0\0\0\0\xfb\xfb\xfb\0\xfb\xfc\xfc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfb\xfa\xfa\0\xfb\xfc\xfc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\x03\xfc\xfc\0\xfe\xfe\xfe\0\0\0\0\0\x04\0\0\0\0\xfc\xfa\xfa\0\xfb\xfa\xfa\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xf9\xf9\xf9\0\xfb\xfa\xfa\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x04\x02\x02\0\0\xff\xff\0\0\0\0\0\x04"
"\0\0\0\0\xfa\xfb\xfb\0\xfa\xfa\xfa\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfa\xf9\xf9\0\xfa\xfa\xfa\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x04\x03\x03\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\xfb\xfb\xfb\0\xfb\xfa\xfa\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfa\xfa\xfa\0\xfb\xfa\xfa\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfa\x05\x05\0\x01\0\0\0\0\0\0\0\x04\0\0\0\0\xfc\xfa\xfa\0\x03\x03\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\x03\x03\0\x03\x03\x03"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xfa\xf8\xf8\0\n"
"\x06\x06\0\0\0\0\0\0\xff\xff\xff\0\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe\0\0\0\xfe%$$\xfe\0\0\0M\x01\xff\xff\xff\0\0\0\0\0\x01\x01"
"\x01M\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\0\0\xff\xff\xb4\xa8\x1d[W1;\xd3\0\0\0\0IEND\xae""B`\x82";

const char* faderalphanonflat_png = (const char*)temp_binary_data_3;


const char* getNamedResource(const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource(const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int)*resourceNameUTF8++;

    switch (hash)
    {
    case 0x96e0bf88:  numBytes = 20692; return faderalphanonflat_png;
    default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "faderalphanonflat_png"
};

const char* originalFilenames[] =
{
    "faderalphanonflat.png"
};

const char* getNamedResourceOriginalFilename(const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename(const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof(namedResourceList) / sizeof(namedResourceList[0])); ++i)
        if (strcmp(namedResourceList[i], resourceNameUTF8) == 0)
            return originalFilenames[i];

    return nullptr;
}

}