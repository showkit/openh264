#include <gtest/gtest.h>
#include <stdlib.h>
#include "codec_api.h"
#include "codec_app_def.h"
//TODO: consider using BaseEncoderTest class from #include "../BaseEncoderTest.h"

class EncoderInterfaceTest : public ::testing::Test {
#define MB_SIZE (16)
#define MAX_WIDTH (3840)
#define MAX_HEIGHT (2160)
 public:
  virtual void SetUp() {
    int rv = WelsCreateSVCEncoder (&pPtrEnc);
    ASSERT_EQ (0, rv);
    ASSERT_TRUE (pPtrEnc != NULL);

    pParamExt = new SEncParamExt();
    ASSERT_TRUE (pParamExt != NULL);

    pSrcPic = new SSourcePicture;
    ASSERT_TRUE (pSrcPic != NULL);

    pOption = new SEncParamExt();
    ASSERT_TRUE (pOption != NULL);

    pYUV = NULL;
    m_iWidth = MAX_WIDTH;
    m_iHeight = MAX_HEIGHT;
    m_iPicResSize =  m_iWidth * m_iHeight * 3 >> 1;
    pYUV = new unsigned char [m_iPicResSize];
    ASSERT_TRUE (pYUV != NULL);
  }

  virtual void TearDown() {
    delete pParamExt;
    delete pOption;
    delete pSrcPic;
    delete []pYUV;
    if (pPtrEnc) {
      WelsDestroySVCEncoder (pPtrEnc);
      pPtrEnc = NULL;
    }
  }

  void TemporalLayerSettingTest();
  void EncodeOneFrame (SEncParamBase* pEncParamBase);
  void PrepareOneSrcFrame();

 public:
  ISVCEncoder* pPtrEnc;

  SEncParamExt* pParamExt;
  SSourcePicture* pSrcPic;
  SEncParamExt* pOption;
  unsigned char* pYUV;

  SFrameBSInfo sFbi;

  int m_iWidth;
  int m_iHeight;
  int m_iPicResSize;
};

void EncoderInterfaceTest::PrepareOneSrcFrame() {
  pSrcPic->iColorFormat = videoFormatI420;
  pSrcPic->uiTimeStamp = 0;
  pSrcPic->iPicWidth = pParamExt->iPicWidth;
  pSrcPic->iPicHeight = pParamExt->iPicHeight;

  for (int i = 0; i < m_iPicResSize; i++)
    pYUV[i] = rand() % 256;

  pSrcPic->iStride[0] = m_iWidth;
  pSrcPic->iStride[1] = pSrcPic->iStride[2] = pSrcPic->iStride[0] >> 1;

  pSrcPic->pData[0] = pYUV;
  pSrcPic->pData[1] = pSrcPic->pData[0] + (m_iWidth * m_iHeight);
  pSrcPic->pData[2] = pSrcPic->pData[1] + (m_iWidth * m_iHeight >> 2);

  //clean the output
  memset (&sFbi, 0, sizeof (SFrameBSInfo));
}

void EncoderInterfaceTest::EncodeOneFrame (SEncParamBase* pEncParamBase) {
  int iResult;

  iResult = pPtrEnc->Initialize (pEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  PrepareOneSrcFrame();
  iResult = pPtrEnc->EncodeFrame (pSrcPic, &sFbi);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  EXPECT_EQ (sFbi.eFrameType, static_cast<int> (videoFrameTypeIDR));

  iResult = pPtrEnc->GetOption (ENCODER_OPTION_SVC_ENCODE_PARAM_BASE, pEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));

  pPtrEnc->Uninitialize();
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
}



void EncoderInterfaceTest::TemporalLayerSettingTest() {

  pParamExt->iPicWidth = m_iWidth;
  pParamExt->iPicHeight = m_iHeight;
  pParamExt->iTargetBitrate = 60000;
  pParamExt->sSpatialLayers[0].iVideoHeight = pParamExt->iPicHeight;
  pParamExt->sSpatialLayers[0].iVideoWidth = pParamExt->iPicWidth;
  pParamExt->sSpatialLayers[0].iSpatialBitrate = 50000;
  pParamExt->iTemporalLayerNum = 1;
  pParamExt->iSpatialLayerNum = 1;

  int iResult = pPtrEnc->InitializeExt (pParamExt);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));

  PrepareOneSrcFrame();

  iResult = pPtrEnc->EncodeFrame (pSrcPic, &sFbi);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  EXPECT_EQ (sFbi.eFrameType, static_cast<int> (videoFrameTypeIDR));

  pSrcPic->uiTimeStamp = 30;
  iResult = pPtrEnc->EncodeFrame (pSrcPic, &sFbi);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  EXPECT_EQ (sFbi.eFrameType, static_cast<int> (videoFrameTypeP));

  memcpy (pOption, pParamExt, sizeof (SEncParamExt));
  pOption ->iTemporalLayerNum = 4;

  ENCODER_OPTION eOptionId = ENCODER_OPTION_SVC_ENCODE_PARAM_EXT;
  iResult = pPtrEnc->SetOption (eOptionId, pOption);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));

  pSrcPic->uiTimeStamp = 60;
  iResult = pPtrEnc->EncodeFrame (pSrcPic, &sFbi);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  EXPECT_EQ (sFbi.eFrameType, static_cast<int> (videoFrameTypeIDR));

  pOption ->iTemporalLayerNum = 2;
  iResult = pPtrEnc->SetOption (eOptionId, pOption);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  pSrcPic->uiTimeStamp = 90;
  iResult = pPtrEnc->EncodeFrame (pSrcPic, &sFbi);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  EXPECT_EQ (sFbi.eFrameType, static_cast<int> (videoFrameTypeP));

  pOption ->iTemporalLayerNum = 4;
  iResult = pPtrEnc->SetOption (eOptionId, pOption);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  pSrcPic->uiTimeStamp = 120;
  iResult = pPtrEnc->EncodeFrame (pSrcPic, &sFbi);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  EXPECT_EQ (sFbi.eFrameType, static_cast<int> (videoFrameTypeP));

  pPtrEnc->Uninitialize();

}

TEST_F (EncoderInterfaceTest, TestTemporalLayerSetting) {
  TemporalLayerSettingTest();
}

void GetValidEncParamBase (SEncParamBase* pEncParamBase) {
  pEncParamBase->iUsageType = CAMERA_VIDEO_REAL_TIME;
  pEncParamBase->iPicWidth = abs ((rand() * 2) + MB_SIZE) % (MAX_WIDTH+1);
  pEncParamBase->iPicHeight = abs ((rand() * 2) + MB_SIZE) % (MAX_HEIGHT+1);
  pEncParamBase->iTargetBitrate = rand() + 1; //!=0
  pEncParamBase->iRCMode = RC_BITRATE_MODE; //-1, 0, 1, 2
  pEncParamBase->fMaxFrameRate = rand() + 0.5f; //!=0
}

TEST_F (EncoderInterfaceTest, BasicInitializeTest) {
  SEncParamBase sEncParamBase;
  GetValidEncParamBase (&sEncParamBase);

  int iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  if (iResult != cmResultSuccess) {
    fprintf (stderr, "Unexpected ParamBase? \
           iUsageType=%d, Pic=%dx%d, TargetBitrate=%d, iRCMode=%d, fMaxFrameRate=%.1f\n",
             sEncParamBase.iUsageType, sEncParamBase.iPicWidth, sEncParamBase.iPicHeight,
             sEncParamBase.iTargetBitrate, sEncParamBase.iRCMode, sEncParamBase.fMaxFrameRate);
  }

  PrepareOneSrcFrame();

  iResult = pPtrEnc->EncodeFrame (pSrcPic, &sFbi);
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
  EXPECT_EQ (sFbi.eFrameType, static_cast<int> (videoFrameTypeIDR));

  pPtrEnc->Uninitialize();
  EXPECT_EQ (iResult, static_cast<int> (cmResultSuccess));
}



TEST_F (EncoderInterfaceTest, BasicInitializeTestFalse) {
  int iResult;
  SEncParamBase sEncParamBase;

  //iUsageType
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.iUsageType = static_cast<EUsageType> (2);
  iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));

  //iPicWidth
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.iPicWidth = 0;
  iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));

  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.iPicWidth = -1;
  iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));

  //TODO: add checking max in interface and then enable this checking
  //GetValidEncParamBase(&sEncParamBase);
  //sEncParamBase.iPicWidth = rand()+(MAX_WIDTH+1);
  //iResult = pPtrEnc->Initialize (&sEncParamBase);
  //EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));

  //iPicHeight
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.iPicHeight = 0;
  iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));

  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.iPicHeight = -1;
  iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));

  //TODO: add checking max in interface and then enable this checking
  //GetValidEncParamBase(&sEncParamBase);
  //sEncParamBase.iPicWidth = rand()+(MAX_HEIGHT+1);
  //iResult = pPtrEnc->Initialize (&sEncParamBase);
  //EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));

  //iTargetBitrate
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.iTargetBitrate = 0;
  iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.iTargetBitrate = -1;
  iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));

  //iUsageType
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.iRCMode = static_cast<RC_MODES> (3);
  iResult = pPtrEnc->Initialize (&sEncParamBase);
  EXPECT_EQ (iResult, static_cast<int> (cmInitParaError));
}

TEST_F (EncoderInterfaceTest, BasicInitializeTestAutoAdjustment) {
  SEncParamBase sEncParamBase;

  // large fMaxFrameRate
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.fMaxFrameRate = 50000;
  EncodeOneFrame (&sEncParamBase);
  EXPECT_LE (sEncParamBase.fMaxFrameRate, 30.0);
  EXPECT_GE (sEncParamBase.fMaxFrameRate, 1.0);

  // fMaxFrameRate = 0
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.fMaxFrameRate = 0;
  EncodeOneFrame (&sEncParamBase);
  EXPECT_LE (sEncParamBase.fMaxFrameRate, 30.0);
  EXPECT_GE (sEncParamBase.fMaxFrameRate, 1.0);

  // fMaxFrameRate = -1
  GetValidEncParamBase (&sEncParamBase);
  sEncParamBase.fMaxFrameRate = -1;
  EncodeOneFrame (&sEncParamBase);
  EXPECT_LE (sEncParamBase.fMaxFrameRate, 30.0);
  EXPECT_GE (sEncParamBase.fMaxFrameRate, 1.0);
}
