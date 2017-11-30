/*
********************************************************************************
*                                    eMOD
*                   the Easy Portable/Player Develop Kits
*                               mod_herb sub-system
*                          (module name, e.g.mpeg4 decoder plug-in) module
*
*          (c) Copyright 2009-2010, Allwinner Microelectronic Co., Ltd.
*                              All Rights Reserved
*
* File   : avi_depack_sequence.h
* Version: V1.0
* By     : Eric_wang
* Date   : 2010-8-11
* Description:
********************************************************************************
*/
#ifndef _AVI_DEPACK_SEQUENCE_H_
#define _AVI_DEPACK_SEQUENCE_H_
//�����8���죬48�ֽ�; 4����,32�ֽ�
typedef struct
{
    cdx_int32  frameIdx;        //��֡������֡�е����(������FFRR�ؼ�֡���е����)����֡һ���ǹؼ�֡,���ڽ���FFRRKeyframeTable.
    cdx_int64  vidChunkOffset;  //for FFRR.���Ե�ַ������ָ��chunk_head!
    cdx_uint32 audPtsArray[MAX_AUDIO_STREAM];  //����������£�ÿ������Ķ�Ӧ��ʱ���(���ų���ʱ��)�����������ο�p->hasAudio.��ǰ�������±�ο�p->CurItemNumInAudioStreamIndexArray
                                               //��¼�����֮֡���һ��audio chunk��Я����PTS.��λ:ms
} OldIdxTableItemT; //Ϊ˳���ȡ��ʽ��Ƶģ�����FFRRKeyframeTable�е�һ��entry�����ݽṹ
extern cdx_int16 AviBuildIdxForOdmlSequenceMode(CdxAviParserImplT *p);
extern cdx_int16 AviBuildIdxForIdx1SequenceMode(CdxAviParserImplT *p);
extern cdx_int32 ReconfigAviReadContextReadMode0(CdxAviParserImplT *p, cdx_uint32 vidTime, 
                    cdx_int32 searchDirection, cdx_int32 searchMode);
extern cdx_int32 ReconfigAviReadContextReadMode1(CdxAviParserImplT *p,
                    cdx_uint32 vidTime, cdx_int32 searchDirection, cdx_int32 searchMode);
extern cdx_int32 CalcAviChunkAudioFrameNum(AudioStreamInfoT *pAudStrmInfo, cdx_int32 nChunkSize);                    
//extern CDX_S32 avi_get_index_by_num_readmode0(struct FILE_PARSER *p, CDX_S32 mode,CDX_U32 *frame_num,CDX_U32 *off_set, CDX_U64 *position,CDX_S32 *diff);
//extern CDX_S32 __reconfig_avi_read_context_readmode0(struct FILE_PARSER *p,CDX_U32 vid_time, CDX_S32 search_direction, CDX_S32 search_mode);
extern cdx_int16 AviReadSequence(CdxAviParserImplT *p);

#endif  /* _AVI_DEPACK_SEQUENCE_H_ */

