#include "wiFont.h"
#include "wiRenderer.h"
#include "wiResourceManager.h"
#include "wiHelper.h"
#include "wiCamera.h"


ID3D11Buffer		*wiFont::vertexBuffer,*wiFont::indexBuffer;
ID3D11InputLayout   *wiFont::vertexLayout;
ID3D11VertexShader  *wiFont::vertexShader;
ID3D11PixelShader   *wiFont::pixelShader;
ID3D11BlendState*		wiFont::blendState;
ID3D11Buffer*           wiFont::constantBuffer;
ID3D11SamplerState*			wiFont::sampleState;
ID3D11RasterizerState*		wiFont::rasterizerState;
ID3D11DepthStencilState*	wiFont::depthStencilState;
UINT wiFont::textlen;
SHORT wiFont::line,wiFont::pos;
BOOL wiFont::toDraw;
DWORD wiFont::counter;
vector<wiFont::Vertex> wiFont::vertexList;
vector<wiFont::wiFontStyle> wiFont::fontStyles;

void wiFont::Initialize()
{
	counter = 0;
	line = pos = 0;
	toDraw=TRUE;
	textlen=0;
	line=pos=0;

	
	indexBuffer=NULL;
	vertexBuffer=NULL;
	vertexLayout=NULL;
	vertexShader=NULL;
	pixelShader=NULL;
	blendState=NULL;
	constantBuffer=NULL;
	sampleState=NULL;
	rasterizerState=NULL;
	depthStencilState=NULL;
}

void wiFont::SetUpStates()
{
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	wiRenderer::graphicsDevice->CreateSamplerState(&samplerDesc, &sampleState);



	
	D3D11_RASTERIZER_DESC rs;
	rs.FillMode=D3D11_FILL_SOLID;
	rs.CullMode=D3D11_CULL_BACK;
	rs.FrontCounterClockwise=TRUE;
	rs.DepthBias=0;
	rs.DepthBiasClamp=0;
	rs.SlopeScaledDepthBias=0;
	rs.DepthClipEnable=FALSE;
	rs.ScissorEnable=FALSE;
	rs.MultisampleEnable=FALSE;
	rs.AntialiasedLineEnable=FALSE;
	wiRenderer::graphicsDevice->CreateRasterizerState(&rs,&rasterizerState);




	
	D3D11_DEPTH_STENCIL_DESC dsd;
	dsd.DepthEnable = false;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;

	dsd.StencilEnable = false;
	dsd.StencilReadMask = 0xFF;
	dsd.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	wiRenderer::graphicsDevice->CreateDepthStencilState(&dsd, &depthStencilState);


	
	D3D11_BLEND_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.RenderTarget[0].BlendEnable=TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	wiRenderer::graphicsDevice->CreateBlendState(&bd,&blendState);
}
void wiFont::SetUpCB()
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	wiRenderer::graphicsDevice->CreateBuffer( &bd, NULL, &constantBuffer );
}
void wiFont::LoadShaders()
{

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	wiRenderer::VertexShaderInfo* vsinfo = static_cast<wiRenderer::VertexShaderInfo*>(wiResourceManager::add("shaders/fontVS.cso", wiResourceManager::VERTEXSHADER, layout, numElements));
	if (vsinfo != nullptr){
		vertexShader = vsinfo->vertexShader;
		vertexLayout = vsinfo->vertexLayout;
	}
	delete vsinfo;


	pixelShader = static_cast<wiRenderer::PixelShader>(wiResourceManager::add("shaders/fontPS.cso", wiResourceManager::PIXELSHADER));









 //   ID3DBlob* pVSBlob = NULL;

	//if(FAILED(D3DReadFileToBlob(L"shaders/fontVS.cso", &pVSBlob))){MessageBox(0,L"Failed To load fontVS.cso",0,0);}
	//wiRenderer::graphicsDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertexShader );
	
	


 //   // Define the input layout
 //   D3D11_INPUT_ELEMENT_DESC layout[] =
 //   {
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
 //   };
	//UINT numElements = ARRAYSIZE( layout );
	//
 //   // Create the input layout
	//wiRenderer::graphicsDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
 //                                         pVSBlob->GetBufferSize(), &vertexLayout );
	//pVSBlob->Release();

 //   

	//
	//ID3DBlob* pPSBlob = NULL;

	//if(FAILED(D3DReadFileToBlob(L"shaders/fontPS.cso", &pPSBlob))){MessageBox(0,L"Failed To load fontPS.cso",0,0);}
	//wiRenderer::graphicsDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixelShader );
	//pPSBlob->Release();
}
void wiFont::SetUpStaticComponents()
{
	SetUpStates();
	SetUpCB();
	LoadShaders();
	LoadVertexBuffer();
	LoadIndices();
}
void wiFont::CleanUpStatic()
{
	for(unsigned int i=0;i<fontStyles.size();++i) 
		fontStyles[i].CleanUp();
	fontStyles.clear();

	vertexList.clear();
	if(vertexBuffer) vertexBuffer->Release();	vertexBuffer=NULL;
	if(indexBuffer) indexBuffer->Release();		indexBuffer=NULL;
	if(vertexShader) vertexShader->Release();	vertexShader=NULL;
	if(pixelShader) pixelShader->Release();		pixelShader=NULL;
	if(vertexLayout) vertexLayout->Release();	vertexLayout=NULL;
	if(constantBuffer) constantBuffer->Release();	constantBuffer=NULL;
	if(sampleState) sampleState->Release();		sampleState=NULL;
	if(rasterizerState) rasterizerState->Release();	rasterizerState=NULL;
	if(blendState) blendState->Release();		blendState=NULL;
	if(depthStencilState) depthStencilState->Release();	depthStencilState=NULL;
}


void wiFont::ModifyGeo(const wchar_t* text, XMFLOAT2 sizSpa,const int& style, ID3D11DeviceContext* context)
{
	textlen=wcslen(text);
	line=0; pos=0;
	vertexList.resize(textlen*4);
	for(unsigned int i=0;i<vertexList.size();i++){
		vertexList[i].Pos=XMFLOAT2(0,0);
		vertexList[i].Tex=XMFLOAT2(0,0);
	}
	for (unsigned int i = 0; i<vertexList.size(); i += 4){

		FLOAT leftX=0.0f,rightX=(FLOAT)fontStyles[style].charSize,upperY=0.0f,lowerY=(FLOAT)fontStyles[style].charSize;
		BOOL compatible=FALSE;
		
		
		if(text[i/4]==10){ //line break
			line+=(SHORT)fontStyles[style].recSize;
			pos=0;
		}
		else if(text[i/4]==32){ //space
			pos+=(SHORT)(fontStyles[style].recSize+sizSpa.x+sizSpa.y);
		}
		else if(fontStyles[style].lookup[text[i/4]].code==text[i/4]){
			leftX+=fontStyles[style].lookup[text[i/4]].offX*(FLOAT)fontStyles[style].charSize;
			rightX+=fontStyles[style].lookup[text[i/4]].offX*(FLOAT)fontStyles[style].charSize;
			upperY+=fontStyles[style].lookup[text[i/4]].offY*(FLOAT)fontStyles[style].charSize;
			lowerY+=fontStyles[style].lookup[text[i/4]].offY*(FLOAT)fontStyles[style].charSize;
			compatible=TRUE;
		}

		if(compatible){
			leftX/=(FLOAT)fontStyles[style].texWidth;
			rightX/=(FLOAT)fontStyles[style].texWidth;
			upperY/=(FLOAT)fontStyles[style].texHeight;
			lowerY/=(FLOAT)fontStyles[style].texHeight;

			vertexList[i].Pos=XMFLOAT2(pos+0-sizSpa.x*0.5f,0-line+sizSpa.x*0.5f); vertexList[i].Tex=XMFLOAT2(leftX,upperY);
			vertexList[i+1].Pos=XMFLOAT2(pos+fontStyles[style].recSize+sizSpa.x*0.5f,0-line+sizSpa.x*0.5f); vertexList[i+1].Tex=XMFLOAT2(rightX,upperY);
			vertexList[i+2].Pos=XMFLOAT2(pos+0-sizSpa.x*0.5f,-fontStyles[style].recSize-line-sizSpa.x*0.5f); vertexList[i+2].Tex=XMFLOAT2(leftX,lowerY);
			vertexList[i+3].Pos=XMFLOAT2(pos+fontStyles[style].recSize+sizSpa.x*0.5f,-fontStyles[style].recSize-line-sizSpa.x*0.5f); vertexList[i+3].Tex=XMFLOAT2(rightX,lowerY);

			pos+=(SHORT)(fontStyles[style].recSize+sizSpa.x+sizSpa.y);
		}
	}
	//wiRenderer::getImmediateContext()->UpdateSubresource( vertexBuffer, 0, NULL, vertexList.data(), 0, 0 );

	wiRenderer::UpdateBuffer(vertexBuffer,vertexList.data(),context==nullptr?wiRenderer::getImmediateContext():context,sizeof(Vertex) * textlen * 4);
	//D3D11_MAPPED_SUBRESOURCE mappedResource;
	//Vertex* dataPtr;
	//wiRenderer::getImmediateContext()->Map(vertexBuffer,0,D3D11_MAP_WRITE_DISCARD,0,&mappedResource);
	//dataPtr = (Vertex*)mappedResource.pData;
	//memcpy(dataPtr,vertexList.data(),sizeof(Vertex) * textlen * 4);
	//wiRenderer::getImmediateContext()->Unmap(vertexBuffer,0);
}

void wiFont::LoadVertexBuffer()
{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof( Vertex ) * MAX_TEXT * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		wiRenderer::graphicsDevice->CreateBuffer( &bd, NULL, &vertexBuffer );
}
void wiFont::LoadIndices()
{
	std::vector<unsigned long>indices;
	indices.resize(MAX_TEXT*6);
	for(unsigned long i=0;i<MAX_TEXT*4;i+=4){
		indices[i/4*6+0]=i/4*4+0;
		indices[i/4*6+1]=i/4*4+2;
		indices[i/4*6+2]=i/4*4+1;
		indices[i/4*6+3]=i/4*4+1;
		indices[i/4*6+4]=i/4*4+2;
		indices[i/4*6+5]=i/4*4+3;
	}
	
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( unsigned long ) * MAX_TEXT * 6;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = indices.data();
	wiRenderer::graphicsDevice->CreateBuffer( &bd, &InitData, &indexBuffer );
}


void wiFont::DrawBlink(wchar_t* text, XMFLOAT4 newPosSizSpa, const char* Halign, const char* Valign, wiRenderer::DeviceContext context)
{
	if(toDraw){
		Draw(text,newPosSizSpa,Halign,Valign,context);
	}
}
void wiFont::DrawBlink(const std::string& text, XMFLOAT4 newPosSizSpa, const char* Halign, const char* Valign, wiRenderer::DeviceContext context)
{
	if(toDraw){
		Draw(text,newPosSizSpa,Halign,Valign,context);
	}
}
void wiFont::Draw(const std::string& text, XMFLOAT4 newPosSizSpa, const char* Halign, const char* Valign, wiRenderer::DeviceContext context)
{
	std::wstring ws(text.begin(), text.end());

	Draw(ws.c_str(),newPosSizSpa,Halign,Valign,context);
}
void wiFont::Draw(const wchar_t* text, XMFLOAT4 newPosSizSpa, const char* Halign, const char* Valign, wiRenderer::DeviceContext context)
{
	Draw(text,"",newPosSizSpa,Halign,Valign,context);
}

void wiFont::DrawBlink(const string& text,const char* fontStyle,XMFLOAT4 newPosSizSpa, const char* Halign, const char* Valign, wiRenderer::DeviceContext context){
	if(toDraw){
		Draw(text,fontStyle,newPosSizSpa,Halign,Valign,context);
	}
}
void wiFont::DrawBlink(const wchar_t* text,const char* fontStyle,XMFLOAT4 newPosSizSpa, const char* Halign, const char* Valign, wiRenderer::DeviceContext context){
	if(toDraw){
		Draw(text,fontStyle,newPosSizSpa,Halign,Valign,context);
	}
}
void wiFont::Draw(const string& text,const char* fontStyle,XMFLOAT4 newPosSizSpa, const char* Halign, const char* Valign, wiRenderer::DeviceContext context){
	wstring ws(text.begin(),text.end());
	Draw(ws.c_str(),fontStyle,newPosSizSpa,Halign,Valign,context);
}


void wiFont::Draw(const wchar_t* text,const char* fontStyle,XMFLOAT4 newPosSizSpa, const char* Halign, const char* Valign, wiRenderer::DeviceContext context){
	int fontStyleI = getFontStyleByName(fontStyle);


	if(!strcmp(Halign,"center") || !strcmp(Valign,"mid"))
		newPosSizSpa.x-=textWidth(text,newPosSizSpa.w+newPosSizSpa.z,fontStyleI)/2;
	else if(!strcmp(Halign,"right"))
		newPosSizSpa.x-=textWidth(text,newPosSizSpa.w+newPosSizSpa.z,fontStyleI);
	if(!strcmp(Valign,"center") || !strcmp(Valign,"mid"))
		newPosSizSpa.y+=textHeight(text,newPosSizSpa.z,fontStyleI)*0.5f;
	else if(!strcmp(Valign,"bottom"))
		newPosSizSpa.y+=textHeight(text,newPosSizSpa.z,fontStyleI);

	
	ModifyGeo(text,XMFLOAT2(newPosSizSpa.z,newPosSizSpa.w),fontStyleI,context);

	if(textlen){

		if(context==nullptr)
			context=wiRenderer::getImmediateContext();
	
		wiRenderer::BindPrimitiveTopology(wiRenderer::PRIMITIVETOPOLOGY::TRIANGLELIST,context);
		wiRenderer::BindVertexLayout(vertexLayout,context);
		wiRenderer::BindVS(vertexShader,context);
		wiRenderer::BindPS(pixelShader,context);


		ConstantBuffer* cb = new ConstantBuffer();
		cb->mProjection = XMMatrixTranspose( wiRenderer::getCamera()->Oprojection );
		cb->mTrans =  XMMatrixTranspose( XMMatrixTranslation(newPosSizSpa.x,newPosSizSpa.y,0) );
		cb->mDimensions = XMFLOAT4((float)wiRenderer::RENDERWIDTH, (float)wiRenderer::RENDERHEIGHT, 0, 0);
		
		wiRenderer::UpdateBuffer(constantBuffer,cb,context);
		delete cb;

		wiRenderer::BindConstantBufferVS(constantBuffer,0,context);

		wiRenderer::BindRasterizerState(rasterizerState,context);
		wiRenderer::BindDepthStencilState(depthStencilState,1,context);

		wiRenderer::BindBlendState(blendState,context);
		wiRenderer::BindVertexBuffer(vertexBuffer,0,sizeof(Vertex),context);
		wiRenderer::BindIndexBuffer(indexBuffer,context);

		wiRenderer::BindTexturePS(fontStyles[fontStyleI].texture,0,context);
		wiRenderer::BindSamplerPS(sampleState,0,context);
		wiRenderer::DrawIndexed(textlen*6,context);
	}
}

void wiFont::Blink(DWORD perframe,DWORD invisibleTime)
{
	counter++;
	if(toDraw && counter>perframe){
		counter=0;
		toDraw=FALSE;
	}
	else if(!toDraw && counter>invisibleTime){
		counter=0;
		toDraw=TRUE;
	}
}


int wiFont::textWidth(const wchar_t* text,FLOAT spacing,const int& style)
{
	int i=0;
	int max=0,lineW=0;
	int len=wcslen(text);
	while(i<len){
		if(text[i]==10) {//ENDLINE
			if(max<lineW) max=lineW;
			lineW=0;
		}
		else lineW++;
		i++;
	}
	if(max==0) max=lineW;

	return (int)(max*(fontStyles[style].recSize+spacing));
}
int wiFont::textHeight(const wchar_t* text,FLOAT siz,const int& style)
{
	int i=0;
	int lines=1;
	int len=wcslen(text);
	while(i<len){
		if(text[i]==10) {//ENDLINE
			lines++;
		}
		i++;
	}

	return (int)(lines*(fontStyles[style].recSize+siz));
}




wiFont::wiFontStyle::wiFontStyle(const string& newName){

	wiRenderer::Lock();

	name=newName;

	for(short i=0;i<127;i++) lookup[i].code=lookup[i].offX=lookup[i].offY=0;

	std::stringstream ss(""),ss1("");
	ss<<"fonts/"<<name<<".txt";
	ss1<<"fonts/"<<name<<".dds";
	std::ifstream file(ss.str());
	if(file.is_open()){
		texture = (wiRenderer::TextureView)wiResourceManager::add(ss1.str());
		file>>texWidth>>texHeight>>recSize>>charSize;
		int i=0;
		while(!file.eof()){
			i++;
			int code=0;
			file>>code;
			lookup[code].code=code;
			file>>lookup[code].offX>>lookup[code].offY>>lookup[code].width;
		}
		file.close();
	}
	else {
		wiHelper::messageBox(name,"Could not load Font Data!"); 
	}

	wiRenderer::Unlock();
}
void wiFont::wiFontStyle::CleanUp(){
	if(texture) texture->Release(); texture=NULL;
}
void wiFont::addFontStyle( const string& toAdd ){
	fontStyles.push_back(wiFontStyle(toAdd));
}
int wiFont::getFontStyleByName( const string& get ){
	for (unsigned int i = 0; i<fontStyles.size(); i++)
	if(!fontStyles[i].name.compare(get))
		return i;
	return 0;
}