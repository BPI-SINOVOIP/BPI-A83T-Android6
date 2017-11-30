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

********************************************************************************
*/
#ifndef _AVI_IDX1_H_
#define _AVI_IDX1_H_
typedef struct
{
    cdx_uint32        ckid;  //�ļ������ֽ�˳�����У���cdx_uint32����,
    cdx_uint32        dwFlags;
    cdx_uint32        dwChunkOffset;        // Position of chunk
    cdx_uint32        dwChunkLength;        // Length of chunk
} AviIndexEntryT;    //idx1���ļ���ʽ

//struct index_table_reader{
//            cdx_int32   left_idx_item;      //all left idx item number.
//            cdx_int32   buf_idx_item;       //the filebuffer contain the idx item number. It is a counter.
//            cdx_int32   idx_counter;        //count all the index item.
//            cdx_int32   idx1_start;         //idx1 offset from file start.
//        
//            struct cdx_stream_info    *fp;                //in common, fp has already point to idx_start.
//            cdx_uint8    *filebuffer;        //used to store index. length:INDEX_CHUNK_BUFFER_SIZE, malloc memory already.
//            AVIINDEXENTRY   *pidxentry; //point to filebuffer.
//            cdx_int32   stream_idx;         // indicate which stream's chunk, we want to search.
//            cdx_int64   chunk_total_size;   //it is the size of all readed chunk length
//            cdx_int64   chunk_counter;      //it is the chunk number of all readed chunk of one stream.
//        
            //cdx_uint32   aud_pts;            //last audio chunk's PTS
            //cdx_uint32   aud_chunk_offset;   //last audio chunk's offset(from file start)
//            cdx_int32   aud_chunk_idx;          //last audio chunk idx, audio chunk counter.
//            cdx_int64   aud_chunk_index_offset; //last audio chunk's index item's offset.
//            cdx_int64   aud_total_chunk_size;   //before last audio chunk's all audio chunks' total size.
//};
struct PsrIdx1TableReader{
    cdx_int32   leftIdxItem;      //all left idx item number.�ļ���ʣ�µĻ�û�ж���reader��filebuffer�ĵ�idx1 entry������
    cdx_int32   bufIdxItem;       //the filebuffer contain the idx item number. It is a counter.��ʾfilebuffer�л�ʣ�¶���idx1 entryû�ж�ȡ

    CdxStreamT    *idxFp;            //in common, fp has already point to idx_start.
    cdx_int64   fpPos;             //indicate the location of current idx in file.
    cdx_uint8    *fileBuffer;        //used to store index. length:INDEX_CHUNK_BUFFER_SIZE, malloc memory already.
    AviIndexEntryT *pIdxEntry; //point to filebuffer.
    cdx_int32   streamIdx;         // indicate which stream's chunk, we want to search.ָʾ��ȡ�ĸ�stream��idx1��
    cdx_uint32   ckBaseOffset;       //avi file index entry indicate the  chunk offset, ckBaseOffset indicate this chunk'offset's, start address(base on the file start).
    cdx_int32   readEndFlg;       //0:not read end, 1:read end index table.

    //��ȡindex��ʱ�ļ�¼����Щ���Ƕ�idx1��ó���ͳ�����ݡ����Ŷ�����ʱ,����������Щͳ������ʹ��avi_in->uAudioPtsArray[]��3������
    cdx_int32   chunkCounter;      //������,��¼�����õ㿪ʼ���˶��ٸ�chunk
    cdx_int32   chunkSize;         //��ǰ������chunk��body size.
    cdx_int64   chunkIndexOffset; //last audio chunk's index item's offset.����¼��chunk��idx1���е�entry����ʼ��ַ��������ffrrkeyframetable������
    cdx_int64   totalChunkSize;   //before last audio chunk's all audio chunks' total size.��¼�����õ㿪ʼ���Ѷ���chunk�����ֽ���
};
//extern cdx_int32 search_next_idx1_index_entry(struct psr_idx1_table_reader *preader, AVI_CHUNK_POSITION *pChunkPos);
//extern CDX_S16 AVI_build_idx_for_index_mode(struct FILE_PARSER *p);
//extern cdx_int32 initial_psr_idx1_table_reader(struct psr_idx1_table_reader *preader, AVI_FILE_IN *avi_in, cdx_int32 stream_index);
//extern void deinitial_psr_idx1_table_reader(struct psr_idx1_table_reader *preader);
#endif  /* _AVI_IDX1_H_ */

