/****************************************************************************/
/*																			*/
/*	MPEG4core Cmodel														*/
/*																			*/
/*	Copyright (C) Renesas Technology Corp., 2003. All rights reserved.		*/
/*																			*/
/*	Version  1.0 : m4vac_vlc.c				2003/03/11 12:00				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*	Modification history													*/
/*	Ver.1.00	2003/03/11	start codes										*/
/*																			*/
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "m4vac_vlc.h"
#include "m4vac_common.h"
#include "m4vac_debug.h"
#include "m4vac_scan.h"		// 03/02/17 added
#include "m4vac_pred.h"		// 03/02/17 added
#include "m4vac_vlc_table_miche.h"	//03/09/26 added
#ifdef iVCP1_VLC_HM
#include "m4vae.h"
#include "mpeg4venc.h"
#endif

#ifdef iVCP1_VLC_HM
extern m4vae_vol_info vol_info;
#endif
extern m4vac_vlc_mbinfo vlc_mbinfo;

/******************************************************************************
*
* Function Name	: m4avc_slice_trailing
*
*****************************************************************************/

int m4vac_slice_trailing()
{
    int offset, len;

    offset = getoffset();						//get offset_bit
    len = ((32 - (offset + 1)) & 0x7) + 1;

#ifdef iVCP1_VLC_HM
    len = putbits((128 >> (8 - len)), len);	//put next_start_code
    //if((offset % 8) == 0)
    //    putbits( 0, 23);    //flush ???
    return len;
#else
    return putbits((128 >> (8 - len)), len);	//put next_start_code
#endif
}

#ifdef iVCP1_VLC_HM
int m4vac_get_offset()
{
    int offset, len;

    offset = getoffset();						//get offset_bit
    len = ((32 - (offset + 1)) & 0x7) + 1;
    return len;
}
/******************************************************************************
*
* Function Name	: m4avc_filler
*
*****************************************************************************/

int m4vac_filler(int size)
{
    int len = 0, offset, align_len;
    //start code
    len += putbits(0x00000001, 32);
    //header
    len += putbits(0x0c, 8);

    //0xFF byte
    while (size >= 4) {
        len += putbits(0xFFFFFFFF, 32);
        size -= 4;
    }
    //remain byte
    if(size != 0)
        len += putbits(0xFFFFFFFF, size << 3);

    //trailing one & alignment
    offset = getoffset();						//get offset_bit
    align_len = ((32 - (offset + 1)) & 0x7) + 1;
    len += putbits((128 >> (8 - align_len)), align_len);
    return len;
}
#endif
/******************************************************************************
*
* Function Name	: m4avc_alignment_zero
*
*****************************************************************************/

int m4vac_alignment_zero()
{
    int offset, len;

    offset = getoffset();						//get offset_bit

    //len = ((32 - (offset + 1)) & 0x7) + 1;
    len = (8 - (offset & 0x7)) % 8;

    if (len != 0){
        putbits(0, len);						//put zero_bit
    }

    //	return putbits(0, len);						//put zero_bit
    return len;
}

/******************************************************************************
*
* Function Name	: m4avc_scan_4x4
*
*****************************************************************************/

void m4vac_scan_4x4(
    long *coef_buf,
    long *coef_out, 
    int is_field)
{
    int i;

    if(is_field) {
#ifdef iVCP1E_HM_SPEC
        for (i=0; i<16; i++){
            coef_out[i] = coef_buf[izgzg4x4_field[i]];
        }
#endif
    }else{
        for (i=0; i<16; i++){
            coef_out[i] = coef_buf[izgzg4x4[i]];
        }
    }
}

#ifdef iVCP1_VLC_HM
void m4vac_scan_8x8_cavlc(long *coef_buf, long *coef_out, int is_field) 
{
    int i;

    if(is_field) {
#ifdef iVCP1E_HM_SPEC
        for (i=0; i<64; i++){
            coef_out[i] = coef_buf[izgzg8x8_field_cavlc[i]];
        }
#endif
    }else{
        for (i=0; i<64; i++){
            coef_out[i] = coef_buf[izgzg8x8_cavlc[i]];
        }
    }

}


void m4vac_scan_2x4(
    long *coef_buf,
    long *coef_out)
{
    int i;

    for (i = 0; i < 8; i++){
        coef_out[i] = coef_buf[izgzg2x4[i]];
    }
}
#endif

/******************************************************************************
*
* Function Name	: m4avc_vlc_nC
*
*****************************************************************************/

int m4vac_vlc_nC(
    int nA,
    int nB)
{
    if (nA == TC_UNAVAIL){
        if (nB == TC_UNAVAIL){
            return 0;
        }else{
            return nB;
        }
    }else{
        if (nB == TC_UNAVAIL){
            return nA;
        }else{
            return (nA + nB + 1) >> 1;
        }
    }
}

/******************************************************************************
*
* Function Name	: m4vac_vlc_mb_in_ipslice_avc
*
*****************************************************************************/

int m4vac_vlc_mb_in_ipslice_avc(
    m4vac_vlc_input *vlc_input,
    m4vac_vlc_output *vlc_output,
    //<MZ040729> change slice end flow (fix)
    //int dquant)
    int dquant,
    int slice_over_mb_cnt)
{
    int len, mb_type, intra_mb, intra16x16;
#ifdef iVCP1_VLC_HM
    int intra8x8;
#endif
    int sub_mb_type;
    //<MZ040305> fix multi reference
    int ref_idx_last;
    //<MZ040107> adopt multi reference
    //	int inter16x16, inter8x8, me_type;
    int inter16x16, inter8x8, refidx[4];
    //<MZ040106> add P_16x8, P_8x16
    int inter16x8, inter8x16;
    //<MZ040108> add I_PCM
    int intra_pcm;
    //<MZ040212> adopt constrained_intra_pred
    int constrained_intra_pred;
    long *coef_ptr;
    //<MZ040223> fix I_PCM scan
    int j;
    int i, cbpy, cbpc, mb_posx, mb_posy, mb_sizex, mb_sizey;
    int vop_type, mv_bits, tcoef_bits, not_coded;
    long *cbp_dcac;
    long *coef_in;
    static int mb_skip_run;
    //<MZ040107> improve P_skip
    int vp_enable, vp_mb_num;
    //<MZ040211> fix multi slice encoding
    int vp_mbs_limit;
    int mv[4][2], mvp[4][2];
    int mba_rf, mbb_rf;
    //<MZ040324> exam to improve skip
    static int cbp_zero;
    //    long qp_range = (vlc_input->ctrl_bit_depth_y == 0) ? 51 : 63;    //MZ121022
    long qp_range = 51 + vlc_input->ctrl_bit_depth_y * 12; //TS140804
    int is_field = 0;
    int chroma_format_idc = vlc_input->chroma_format_idc;
#ifdef iVCP1E_HM_SPEC
    is_field = vlc_input->interlaced;
#endif

    len = 0;
    coef_in = (long *)vlc_input->coef_in;
    intra_mb = (vlc_input->me_type == 0);
    //<MZ050418> fix mbinfo output
    if (intra_mb){
        for (i=0; i<4; i++){
            vlc_input->refidx[i] = -1;
            vlc_input->mv_fh0 = vlc_input->mv_fv0 = 0;
            vlc_input->mv_fh1 = vlc_input->mv_fv1 = 0;
            vlc_input->mv_fh2 = vlc_input->mv_fv2 = 0;
            vlc_input->mv_fh3 = vlc_input->mv_fv3 = 0;
            vlc_input->mv_bh0 = vlc_input->mv_bv0 = 0;
            vlc_input->mv_bh1 = vlc_input->mv_bv1 = 0;
            vlc_input->mv_bh2 = vlc_input->mv_bv2 = 0;
            vlc_input->mv_bh3 = vlc_input->mv_bv3 = 0;
        }
    }
    //<MZ040107> add P_16x8, P_8x16
    //	intra16x16 = (intra_mb && vlc_input->intra_16or4);
    intra16x16 = intra_mb && (vlc_input->mb_part==0);
#ifdef iVCP1_VLC_HM
    intra8x8 = intra_mb && (vlc_input->mb_part==2);
#endif
    inter16x16 = !intra_mb && (vlc_input->mb_part==0);
    inter16x8 = !intra_mb && (vlc_input->mb_part==1);
    inter8x16 = !intra_mb && (vlc_input->mb_part==2);
    inter8x8 = !intra_mb & (vlc_input->mb_part==3);
    //<MZ040108> add I_PCM
    intra_pcm = intra_mb && vlc_input->intra_pcm;
    //<MZ040212> adopt constrained_intra_pred
    constrained_intra_pred = vlc_input->constrained_intra_pred;
    //<MZ040305> fix multi reference
    ref_idx_last = vlc_input->ref_idx_last;
    //<MZ040107> adopt multi reference
    //	me_type = vlc_input->me_type;
    for (i=0; i<4; i++){
        refidx[i] = (int)vlc_input->refidx[i];
    }
    cbp_dcac = vlc_input->coef_number;
    mb_posx = vlc_input->mb_posx;
    mb_posy = vlc_input->mb_posy;
    vop_type = vlc_input->vop_type;
    mb_sizex = vlc_input->mb_sizex;
    mb_sizey = vlc_input->mb_sizey;
    //<MZ040107> improve P_skip
    //<MZ050308> unuse vp_enable at H.264
    //	vp_enable = vlc_input->vp_enable;
    vp_enable = 1;
    vp_mb_num = vlc_input->vp_mb_num;
    //<MZ040211> fix multi slice encoding
    vp_mbs_limit = vlc_input->vp_mbs_limit;

    //<MZ041021> output error code
    if (vlc_output->err_code == 0){
        if (vlc_input->coef_clip){
            vlc_output->err_code = ERRE_COEF_CLIP | (mb_posy << 8) | mb_posx;
        }
        //<MZ041029> modify pcm_zero info out
        else if (vlc_input->pcm_zero){
            vlc_output->err_code = ERRE_AVC_PCM_ZERO | (mb_posy << 8) | mb_posx;
        }
        else if ((vop_type == 0) && !intra_mb){
            vlc_output->err_code = ERRE_AVC_MB_TYPE | (mb_posy << 8) | mb_posx;
        }
        //else if (vlc_input->mb_quant > 51){
        else if (vlc_input->mb_quant > qp_range){   //MZ121022
            vlc_output->err_code = ERRE_AVC_MB_QUANT | (mb_posy << 8) | mb_posx;
        }
        else{
            for (i=0; i<4; i++){
                if (vlc_input->refidx[i] > vlc_input->ref_idx_last){
                    vlc_output->err_code = ERRE_AVC_REFIDX | (mb_posy << 8) | mb_posx;
                }
            }
        }
    }

    cbpy = 0;
    if (intra16x16){
        for (i=0; i<4; i++){
            cbpy |= (cbp_dcac[i] & 0x000f);					//check AC
        }
        if (cbpy != 0){
            cbpy = 15;
        }
    }else{
        for (i=0; i<4; i++){
            cbpy = (cbpy | ((cbp_dcac[i] != 0) << i));		//check ACDC
        }
    }

    if (chroma_format_idc == 2) {
        if (((cbp_dcac[4] | cbp_dcac[5] | cbp_dcac[6] | cbp_dcac[7]) & 0x000f) != 0){		//check AC
            cbpc = 2;
        }
        else if (((cbp_dcac[4] | cbp_dcac[5] | cbp_dcac[6] | cbp_dcac[7]) & 0x00f0) != 0){	//check DC
            cbpc = 1;
        }
        else{
            cbpc = 0;
        }
    }
    else {
        if (((cbp_dcac[4] | cbp_dcac[5]) & 0x000f) != 0){		//check AC
            cbpc = 2;
        }
        else if (((cbp_dcac[4] | cbp_dcac[5]) & 0x00f0) != 0){	//check DC
            cbpc = 1;
        }
        else{
            cbpc = 0;
        }
    }

    //<MZ040108> add I_PCM
    if (intra_pcm){
        cbpy = 15;
        cbpc = 2;
    }

    //MZ041220
#ifdef EXAM_FORCE_SKIP2
    printf("cbpy = %d\tcbpc = %d\tintra =%d\n", cbpy, cbpc, intra_mb);
#endif

    vlc_mbinfo.cbp = (cbpc << 4) | cbpy;

    //	not_coded = (!intra_mb && (cbp == 0) && !(interlaced && field_pred) &&
    //			(vlc_input->mv_fh0 == 0) && (vlc_input->mv_fv0 == 0));	//MPEG4
    //	if (vop_type != 0){
    //		len += put_ue(0);
    //	}

    //mb_skip_run
    //<MZ040209> fix multi slice encoding
    //	if ((mb_posx == 0) && (mb_posy == 0)){
    if (vp_mb_num == 0){
        mb_skip_run = 0;
        //<MZ040324> exam to improve skip
        cbp_zero = 0;
    }
    not_coded = 0;
    if (vop_type != 0){
        //<MZ040107> improve P_skip
        /*
        not_coded = (use_mb_skip && (!intra_mb) && (cbpy == 0) && (cbpc == 0) &&
        //<MZ040107> adopt multi reference
        //			(vlc_input->mv_fh0 == 0) && (vlc_input->mv_fv0 == 0) && (vlc_input->me_type == 1) &&
        (vlc_input->mv_fh0 == 0) && (vlc_input->mv_fv0 == 0) && (refidx[0] == 0) &&
        ((mb_posx == 0) || (mb_posy == 0)));		//must add pmv condition
        */
        if (inter16x16 && (cbpy == 0) && (cbpc == 0) && (refidx[0] == 0)){
            // prepare pmv_calc
            mv[0][0] = vlc_input->mv_fh0;
            mv[0][1] = vlc_input->mv_fv0;
            // pmv_calc to judge P_skip
            m4vac_pmv_calc_avc(mv, refidx, mvp,
                mb_posx, mb_posy, vp_enable, vp_mb_num, mb_sizex, 0,
                1, &mba_rf, &mbb_rf);
            // judge P_skip
            if ((mba_rf == RF_INVALID) || (mbb_rf == RF_INVALID) ||
                ((mba_rf == 0) && (mvp[1][0] == 0) && (mvp[1][1] == 0)) ||
                ((mbb_rf == 0) && (mvp[2][0] == 0) && (mvp[2][1] == 0))){
                    if ((vlc_input->mv_fh0 == 0) && (vlc_input->mv_fv0 == 0)){
                        not_coded = 1;
                    }
            }else if ((vlc_input->mv_fh0 == mvp[0][0]) && (vlc_input->mv_fv0 == mvp[0][1])){
                not_coded = 1;
            }
        }

        if (not_coded){
            mb_skip_run++;
            //<MZ031215> add P_8x8
            mb_type = 0;
            //<MZ040211> fix multi slice encoding
#ifdef iVCP1E_HM_SPEC
            if (((mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)) ||
                (vp_mbs_limit != 0 && slice_over_mb_cnt == 4           )    ){
                    VLCLOG("mb_skip_run: %d\n", mb_skip_run);
                    len += put_ue(mb_skip_run);
                    mb_skip_run = 0;
            }
#else
            if (((mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)) ||
                ((vp_mb_num >= vp_mbs_limit - 1) && (vp_mbs_limit != 0)) ||
                (slice_over_mb_cnt == 3)){
                    VLCLOG("mb_skip_run: %d\n", mb_skip_run);
                    len += put_ue(mb_skip_run);
                    mb_skip_run = 0;
            }
#endif
        }else{
            VLCLOG("mb_skip_run: %d\n", mb_skip_run);
            len += put_ue(mb_skip_run);
            mb_skip_run = 0;
        }
    }
    //<MZ040108> improve P_skip
    vlc_output->not_coded = not_coded;
#ifdef iVCP1E_HM_SPEC
    vlc_output->mb_skip_run = mb_skip_run;
#endif

    if ((vop_type != 0) && not_coded){
        //<MZ031215> add P_8x8
        //		m4vac_vlc_mv_avc(vlc_input, vlc_output, 2);					//pmv update
        m4vac_vlc_mv_avc(vlc_input, vlc_output, mb_type, 2);					//pmv update
        //<MZ040212> adopt constrained_intra_pred
        //		m4vac_vlc_intra4x4(vlc_input, vlc_output, 1);				//intra4x4mode update only
        if (constrained_intra_pred){
            m4vac_vlc_intra4x4(vlc_input, vlc_output, 2);			//intra4x4mode update only
        }
        else{
            m4vac_vlc_intra4x4(vlc_input, vlc_output, 1);			//intra4x4mode update only
        }
        //<MZ040209> fix multi slice encoding
        //		m4vac_residual(coef_in, 0, 0, 0, mb_posx, mb_posy, 1);		//total_coef update_only
        m4vac_residual(coef_in, 0, 0, 0, mb_posx, mb_posy,
            mb_sizex, vp_enable, vp_mb_num, is_field, chroma_format_idc, 1);			//total_coef update_only
        vlc_output->q_not_coded = 1;
        mv_bits = 0;
        tcoef_bits = 0;

        VLCLOG("MB(%d): mb_skip\n", mb_sizex*mb_posy + mb_posx);
        VLCLOG("  MV (%ld, %ld) = MVP (%d, %d) + MVD (0, 0)\n",
            vlc_input->mv_fh0, vlc_input->mv_fv0, mvp[0][0], mvp[0][1]);
        //<MZ040324> exam to improve skip
        //printf("..... SKIP MB(%d, %d)\tSAD = %d\tQP = %d\tMXD = %d\n", mb_posy, mb_posx, vlc_input->sad, vlc_input->mb_quant, vlc_input->mxd);

        //<MZ040108> add I_PCM
    }else if (intra_pcm){
        //mb_type
        mb_type = 25;
        if (vop_type == 1) mb_type += 5;
        VLCLOG("MB(%ld): mb_type = %d\n", vlc_input->mb_sizex*mb_posy + mb_posx, mb_type);
        len += put_ue(mb_type);
        //update
        //<MZ040212> adopt constrained_intra_pred
        //		m4vac_vlc_intra4x4(vlc_input, vlc_output, 1);				//intra4x4mode update only
        if (constrained_intra_pred){
            m4vac_vlc_intra4x4(vlc_input, vlc_output, 2);			//intra4x4mode update only
        }
        else{
            m4vac_vlc_intra4x4(vlc_input, vlc_output, 1);			//intra4x4mode update only
        }
        if (vop_type == 1){
            m4vac_vlc_mv_avc(vlc_input, vlc_output, mb_type, 1);	//intra, update only
        }
        //<MZ040209> fix multi slice encoding
        //		m4vac_residual(coef_in, 0, cbpy, cbpc, mb_posx, mb_posy, 2);		//total_coef update_only
        m4vac_residual(coef_in, 0, cbpy, cbpc, mb_posx, mb_posy,
            mb_sizex, vp_enable, vp_mb_num, is_field, chroma_format_idc, 2);			//total_coef update_only
        vlc_output->q_not_coded = 1;
        mv_bits = 0;
        tcoef_bits = 0;
        //pcm_alignment_zero_bit
        len += m4vac_alignment_zero();
        //<MZ040223> fix I_PCM scan
        //		//pcm_byte
        //		coef_ptr = coef_in;
        //		for(i=0; i<384; i++){
        //			len += putbits(*(coef_ptr++), 8);
        //		}
        //pcm_byte Y
        for(i=0; i<32; i++){
            coef_ptr = coef_in + ((i%16)/2 + (i%2)*8 + (i/16)*16) * 8;
            for(j=0; j<8; j++){
                //<MZ041021> output error code
                if ((vlc_output->err_code == 0) && (*coef_ptr == 0)){
                    vlc_output->err_code = ERRE_AVC_PCM_ZERO | (mb_posy << 8) | mb_posx;
                }
                len += putbits(*(coef_ptr++), 8);
            }
        }
        //pcm_byte C
        for(i=0; i<16; i++){
            coef_ptr = coef_in + (i + 32) * 8;
            for(j=0; j<8; j++){
                //<MZ041021> output error code
                if ((vlc_output->err_code == 0) && (*coef_ptr == 0)){
                    vlc_output->err_code = ERRE_AVC_PCM_ZERO | (mb_posy << 8) | mb_posx;
                }
                len += putbits(*(coef_ptr++), 8);
            }
        }

    }else{
        //mb_type
        mb_type = 0;
        if (intra_mb){
            if (vop_type == 1){
                mb_type += 5;
            }
            if (intra16x16){
                mb_type++;
                if (cbpy){
                    mb_type += 12;
                }
                mb_type += cbpc * 4;
                mb_type += vlc_input->intra_luma_type[0];
            }
        }
        //<MZ040106> add P_16x8, P_8x16
        else if (inter16x8){
            mb_type++;			//P_16x8
        }
        else if (inter8x16){
            mb_type += 2;		//P_8x16
        }
        //<MZ031210> add P_8x8
        else if (inter8x8){
            //<MZ040305> fix multi reference
            //if ((refidx[0] == 0) && (refidx[1] == 0) && (refidx[2] == 0) && (refidx[3] == 0)){
#ifdef iVCP1E2_ME_SPEC
            mb_type += 4;		//P_8x8ref0
#else
            if ((ref_idx_last != 0) && (refidx[0] == 0) && (refidx[1] == 0) && (refidx[2] == 0) && (refidx[3] == 0)){
                mb_type += 4;		//P_8x8ref0
            }else{
                mb_type += 3;		//P_8x8
            }
#endif
        }
        VLCLOG("MB(%ld): mb_type = %d\n", vlc_input->mb_sizex*mb_posy + mb_posx, mb_type);
        len += put_ue(mb_type);

        //transform_size_8x8_flag
#ifdef iVCP1_VLC_HM
        //if(vol_info.sps_profile_idc > iVCP1_EXTENDED && vol_info.transform8x8_en) {
        if(vol_info.sps_profile_idc > iVCP1_EXTENDED && vol_info.transform8x8_en && !intra16x16 && intra_mb) {
            VLCLOG("  transform_size_8x8_flag = %d\n", intra8x8);
            len += putbits(intra8x8 & 0x1, 1);
        }else if(intra_mb) {
            vlc_input->transform_size_8x8_flag = 0;
        }
#endif
        //mb_pred
        mv_bits = 0;
        if (intra_mb){
#ifdef iVCP1_VLC_HM
            if(intra8x8){
                //intra8x8_pred
                mv_bits += m4vac_vlc_intra4x4(vlc_input, vlc_output, 3);	//intra8x8
            }else if (intra16x16 == 0){
                //intra4x4_pred
                mv_bits += m4vac_vlc_intra4x4(vlc_input, vlc_output, 0);	//intra4x4
                vlc_output->intra4x4_mbs++;
                for (i=0; i<16; i++){
                    vlc_output->intra4x4_type[vlc_input->intra_luma_type[i]]++;
                }
            }else{
                m4vac_vlc_intra4x4(vlc_input, vlc_output, 1);	//intra16x16, update only
                vlc_output->intra16x16_mbs++;
                vlc_output->intra16x16_type[vlc_input->intra_luma_type[0]]++;
            }
#else
            //intra4x4_pred
            if (intra16x16 == 0){
                mv_bits += m4vac_vlc_intra4x4(vlc_input, vlc_output, 0);	//intra4x4
                vlc_output->intra4x4_mbs++;
                for (i=0; i<16; i++){
                    vlc_output->intra4x4_type[vlc_input->intra_luma_type[i]]++;
                }
            }else{
                m4vac_vlc_intra4x4(vlc_input, vlc_output, 1);	//intra16x16, update only
                vlc_output->intra16x16_mbs++;
                vlc_output->intra16x16_type[vlc_input->intra_luma_type[0]]++;
            }
#endif

            //intra_chroma_pred
            if(vlc_input->chroma_format_idc == 1 || vlc_input->chroma_format_idc == 2) {
                VLCLOG("  intra_chroma_pred_mode: %ld\n", vlc_input->intra_chroma_type);
                mv_bits += put_ue(vlc_input->intra_chroma_type);
                vlc_output->intra_chroma_type[vlc_input->intra_chroma_type]++;
            }

            if (vop_type == 1){
                //<MZ031215> add P_8x8
                //				m4vac_vlc_mv_avc(vlc_input, vlc_output, 1);		//intra, update only
                m4vac_vlc_mv_avc(vlc_input, vlc_output, mb_type, 1);		//intra, update only
            }
        }else{
            //<MZ031210> add P_8x8
            if (inter8x8){
                //sub_mb_type
                VLCLOG("  sub_mb_type: ");
                for (i=0; i<4; i++){
                    sub_mb_type = 0;	//always P_L0_8x8
                    VLCLOG("%d ", sub_mb_type);
                    len += put_ue(sub_mb_type);
                }
                VLCLOG("\n");
                /* ref_idx and mvd of sub MB P_L0_8x8 can be decoded by the same operation with P_L0_16x16 */
            }
            //<MZ040212> adopt constrained_intra_pred
            //			m4vac_vlc_intra4x4(vlc_input, vlc_output, 1);		//inter, update only
            if (constrained_intra_pred){
                m4vac_vlc_intra4x4(vlc_input, vlc_output, 2);	//inter, update only
            }
            else{
                m4vac_vlc_intra4x4(vlc_input, vlc_output, 1);	//inter, update only
            }
            //<MZ031210> add P_8x8
            //mv_bits += m4vac_vlc_mv_avc(vlc_input, vlc_output, 0);		//inter
            mv_bits += m4vac_vlc_mv_avc(vlc_input, vlc_output, mb_type, 0);		//inter
        }
        len += mv_bits;

        //coded_block_pattern
        if (intra16x16 == 0){
            VLCLOG("  coded_block_pattern : %02x\n", (cbpc << 4) | cbpy);
            len += put_me((cbpc << 4) | cbpy, intra_mb, vlc_input->chroma_format_idc);
            //<MZ040324> exam to improve skip
            if (!intra_mb && (cbpc==0) && (cbpy==0)){
                cbp_zero++;
                //printf("CBP0_MBs = %d\tSAD = %d\tQP = %d\n", cbp_zero, vlc_input->cme_sad, vlc_input->mb_quant);
            }
            //transform_size_8x8_flag
#ifdef iVCP1_VLC_HM
            if(vol_info.sps_profile_idc > iVCP1_EXTENDED && vol_info.transform8x8_en && !intra_mb && cbpy != 0) { //FIXME
                VLCLOG("  transform_size_8x8_flag = %ld\n", vlc_input->transform_size_8x8_flag);
                len += putbits(vlc_input->transform_size_8x8_flag & 0x1, 1);
            }else if(!intra_mb && vlc_input->transform_size_8x8_flag){
                vlc_input->transform_size_8x8_flag = 0;
            }
#endif
        }

        //mb_qp_delta
        if (intra16x16 || cbpy || cbpc){
            //MZ121023
            if (vlc_input->ctrl_bit_depth_y == 0){
                VLCLOG("  mb_qp_delta : %d  (%ld)\n", dquant, vlc_input->mb_quant);
            }
            else{
                // consider 10bit only
                VLCLOG("  mb_qp_delta : %d  (%ld)\n", dquant, vlc_input->mb_quant - vlc_input->ctrl_bit_depth_y*12);
            }
            len += put_se(dquant);
            vlc_output->q_not_coded = 0;

        }else{
            vlc_output->q_not_coded = 1;
        }

        //residual()
        //<MZ040209> fix multi slice encoding
        //		tcoef_bits = m4vac_residual(coef_in, intra16x16, cbpy, cbpc, mb_posx, mb_posy, 0);
#ifdef iVCP1_VLC_HM
        tcoef_bits = m4vac_residual(coef_in, ((vlc_input->transform_size_8x8_flag << 1) | intra16x16), cbpy, cbpc, mb_posx, mb_posy,
            mb_sizex, vp_enable, vp_mb_num, is_field, chroma_format_idc, 0);
#else
        tcoef_bits = m4vac_residual(coef_in, intra16x16, cbpy, cbpc, mb_posx, mb_posy,
            mb_sizex, vp_enable, vp_mb_num, is_field, chroma_format_idc, 0);
#endif
        len += tcoef_bits;

        //		if (intra_mb == 0){
        //			VLCLOG("  mv = (%d, %d)\n", vlc_input->mv_fh0, vlc_input->mv_fv0);
        //		}
    }

    if (!intra_mb && !cbpy && !cbpc && !not_coded){
        //printf("..... .... MB(%d, %d): Inter CBP0, MV=(%d, %d)\n",
        //	mb_posy, mb_posx, vlc_input->mv_fh0, vlc_input->mv_fv0);
    }

    if (not_coded){
        vlc_output->notcoded_mbs++;
        vlc_output->pic_skip_mbs++;
    }
    else if (intra_mb){
        vlc_output->intra_mbs++;
        vlc_output->pic_intra_mbs++;
    }
    else {
        vlc_output->inter_mbs++;
    }

    vlc_output->mv_bits += mv_bits;
    vlc_output->tcoef_bits += tcoef_bits;
    return len;
}

/******************************************************************************
*
* Function Name	: m4vac_vlc_pred_i4x4
*
*****************************************************************************/

int m4vac_vlc_pred_i4x4(
    int nA,
    int nB)
{
    int pred_i4x4;

    if ((nA == IT_DCONLY) || (nB == IT_DCONLY)){
        pred_i4x4 = 2;
    }else{
        if (nA == IT_UNAVAIL){
            nA = 2;
        }
        if (nB == IT_UNAVAIL){
            nB = 2;
        }

        if (nA < nB){
            pred_i4x4 = nA;
        }else{
            pred_i4x4 = nB;
        }
    }
    return pred_i4x4;
}

/******************************************************************************
*
* Function Name	: m4vac_vlc_intra4x4
*
*****************************************************************************/

int m4vac_vlc_intra4x4(
    m4vac_vlc_input *vlc_input,
    m4vac_vlc_output *vlc_output,
    int update_only)
{
    int len, i;
    int mb_posx, mb_posy;
    //<MZ040209> fix multi slice encoding
    int mb_sizex, vp_enable, vp_mb_num;
    static int intra_left[4], intra_line[VP4S_MAX_MB_WIDTH][4];	//Y10,Y11,Y14,Y15
    int intra_top[4], intra_mode[16];
    int nA, nB, intra_pred4x4;

    len = 0;
    mb_posx = vlc_input->mb_posx;
    mb_posy = vlc_input->mb_posy;
    //<MZ040209> fix multi slice encoding
    mb_sizex = vlc_input->mb_sizex;
    //<MZ050308> unuse vp_enable at H.264
    //	vp_enable = vlc_input->vp_enable;
    vp_enable = 1;
    vp_mb_num = vlc_input->vp_mb_num;
    //	dc_onlyA = (mb_posx == 0);	//need to add VP, inter & constrained_intra_pred
    //	dc_onlyB = (mb_posy == 0);	//need to add VP, inter & constrained_intra_pred

    //load neighbor data
    if (mb_posx == 0){		//if not, intra_left is set previously.
        for (i=0; i<4; i++){
            intra_left[i] = IT_DCONLY;
        }
    }
    //<MZ040209> fix multi slice encoding
    else if (vp_enable && (vp_mb_num == 0)){		//if not, intra_left is set previously.
        for (i=0; i<4; i++){
            intra_left[i] = IT_DCONLY;
        }
    }
    if (mb_posy == 0){
        for (i=0; i<4; i++){
            intra_top[i] = IT_DCONLY;
        }
        //<MZ040209> fix multi slice encoding
    }else if (vp_enable && (vp_mb_num < mb_sizex)){
        for (i=0; i<4; i++){
            intra_top[i] = IT_DCONLY;
        }
    }else{
        for (i=0; i<4; i++){
            intra_top[i] = intra_line[mb_posx][i];
        }
    }

    if (update_only == 2){			//inter & constrained.
        for (i=0; i<16; i++){
            intra_mode[i] = IT_DCONLY;
        }
    }else if (update_only == 1){	//inter or intra16x16.
        for (i=0; i<16; i++){
            intra_mode[i] = IT_UNAVAIL;
        }
#ifdef iVCP1_VLC_HM
    }else if(update_only == 3) {    //intra8x8
        VLCLOG("  intra8x8_pred_mode:");
        for (i=0; i<4; i++){
            intra_mode[i] = vlc_input->intra_luma_type[i];
            VLCLOG(" %d", intra_mode[i]);
        }
        for (i=8; i<16; i++){
            intra_mode[i] = IT_UNAVAIL;
        }
#endif
    }else{
        VLCLOG("  intra4x4_pred_mode:");
        for (i=0; i<16; i++){
            intra_mode[i] = vlc_input->intra_luma_type[i];
            VLCLOG(" %d", intra_mode[i]);
        }
        VLCLOG("\n");
    }

    if (update_only == 0){
        //Blk8x8 Loop
        for (i=0; i<4; i++){
            //Blk4x4 #0
            if ((i == 0) || (i == 2)){
                nA = intra_left[i];
            }else{
                nA = intra_mode[i*4 - 3];
            }
            if ((i == 0) || (i == 1)){
                nB = intra_top[i*2];
            }else{
                nB = intra_mode[i*4 - 6];
            }

            intra_pred4x4 = m4vac_vlc_pred_i4x4(nA, nB);
            if (intra_mode[i*4] == intra_pred4x4){
                len += putbits(1, 1);		//prev_intra4x4_pred_mode_flag
            }else{
                len += putbits(0, 1);		//prev_intra4x4_pred_mode_flag

                if (intra_mode[i*4] < intra_pred4x4){
                    len += putbits(intra_mode[i*4], 3);
                }else{
                    len += putbits(intra_mode[i*4] - 1, 3);
                }
            }

            //Blk4x4 #1
            nA = intra_mode[i*4];
            if ((i == 0) || (i == 1)){
                nB = intra_top[i*2 + 1];
            }else{
                nB = intra_mode[i*4 - 5];
            }

            intra_pred4x4 = m4vac_vlc_pred_i4x4(nA, nB);
            if (intra_mode[i*4 + 1] == intra_pred4x4){
                len += putbits(1, 1);		//prev_intra4x4_pred_mode_flag
            }else{
                len += putbits(0, 1);		//prev_intra4x4_pred_mode_flag

                if (intra_mode[i*4 + 1] < intra_pred4x4){
                    len += putbits(intra_mode[i*4 + 1], 3);
                }else{
                    len += putbits(intra_mode[i*4 + 1] - 1, 3);
                }
            }

            //Blk4x4 #2
            if ((i == 0) || (i == 2)){
                nA = intra_left[i + 1];
            }else{
                nA = intra_mode[i*4 - 1];
            }
            nB = intra_mode[i*4];

            intra_pred4x4 = m4vac_vlc_pred_i4x4(nA, nB);
            if (intra_mode[i*4 + 2] == intra_pred4x4){
                len += putbits(1, 1);		//prev_intra4x4_pred_mode_flag
            }else{
                len += putbits(0, 1);		//prev_intra4x4_pred_mode_flag

                if (intra_mode[i*4 + 2] < intra_pred4x4){
                    len += putbits(intra_mode[i*4 + 2], 3);
                }else{
                    len += putbits(intra_mode[i*4 + 2] - 1, 3);
                }
            }

            //Blk4x4 #3
            nA = intra_mode[i*4 + 2];
            nB = intra_mode[i*4 + 1];

            intra_pred4x4 = m4vac_vlc_pred_i4x4(nA, nB);
            if (intra_mode[i*4 + 3] == intra_pred4x4){
                len += putbits(1, 1);		//prev_intra4x4_pred_mode_flag
            }else{
                len += putbits(0, 1);		//prev_intra4x4_pred_mode_flag

                if (intra_mode[i*4 + 3] < intra_pred4x4){
                    len += putbits(intra_mode[i*4 + 3], 3);
                }else{
                    len += putbits(intra_mode[i*4 + 3] - 1, 3);
                }
            }
        }
    }
#ifdef iVCP1_VLC_HM
    else if(update_only == 3) { //intra 8x8
        //Blk8x8 Loop
        for (i=0; i<4; i++){
            if ((i == 0) || (i == 2)){
                nA = intra_left[i];
            }else{
                nA = intra_mode[i - 1];
            }
            if ((i == 0) || (i == 1)){
                nB = intra_top[i*2];
            }else{
                nB = intra_mode[i - 2];
            }

            intra_pred4x4 = m4vac_vlc_pred_i4x4(nA, nB);
            if (intra_mode[i] == intra_pred4x4){
                len += putbits(1, 1);		//prev_intra4x4_pred_mode_flag
            }else{
                len += putbits(0, 1);		//prev_intra4x4_pred_mode_flag

                if (intra_mode[i] < intra_pred4x4){
                    len += putbits(intra_mode[i], 3);
                }else{
                    len += putbits(intra_mode[i] - 1, 3);
                }
            }
        }
    }
#endif

    //save neighbor data
    for (i=0; i<4; i++){
        intra_left[i] = intra_mode[5 + i/2*8 + i%2*2];
        intra_line[mb_posx][i] = intra_mode[i/2*4 + 10 + i%2];
    }
#ifdef iVCP1_VLC_HM
    //save neighbor data (intra8x8)
    if(update_only == 3) {
        intra_left[0] = intra_mode[1];
        intra_left[1] = intra_mode[1];
        intra_left[2] = intra_mode[3];
        intra_left[3] = intra_mode[3];
        intra_line[mb_posx][0] = intra_mode[2];
        intra_line[mb_posx][1] = intra_mode[2];
        intra_line[mb_posx][2] = intra_mode[3];
        intra_line[mb_posx][3] = intra_mode[3];
    }
#endif

    return len;
}

/******************************************************************************
*
* Function Name	: m4vac_vlc_mv_avc
*
*****************************************************************************/

int m4vac_vlc_mv_avc(
    m4vac_vlc_input *vlc_input,
    m4vac_vlc_output *vlc_output,
    //<MZ031215> add P_8x8
    int mb_type,
    int update_only)
{
    int len;
    int mb_posx, mb_posy, mb_sizex;
    //<MZ031211> add P_8x8
    //	int mv_h, mv_v, mvp_h, mvp_v, mvd_h, mvd_v;
    int mv[4][2], mvp[4][2], mvd[4][2];
    //<MZ031211> add P_8x8
    //	int mv_fh0, mv_fv0, refidx;
    //	int mv_bh0, mv_bv0, me_type;
    //<MZ040107> adopt multi reference
    //	int mv_f[4][2], refidx[4];
    //	int mv_b[4][2], me_type[4];
    int refidx[4];
    //<MZ040218> add ref_idx_last
    int ref_idx_last;
    int blk;
    int vp_enable, vp_mb_num;
    //<MZ040107> improve P_skip
    int mba_rf, mbb_rf;

    mb_posx = vlc_input->mb_posx;
    mb_posy = vlc_input->mb_posy;
    mb_sizex = vlc_input->mb_sizex;
    //<MZ050308> unuse vp_enable at H.264
    //	vp_enable = vlc_input->vp_enable;
    vp_enable = 1;
    vp_mb_num = vlc_input->vp_mb_num;
    //<MZ040218> add ref_idx_last
    ref_idx_last = vlc_input->ref_idx_last;

    //<MZ040107> adopt multi reference
    //<MZ031211> add P_8x8
    //	me_type = vlc_input->me_type;
    //	for (blk=0; blk<4; blk++){
    //		me_type[blk] = vlc_input->me_type;		//interim
    //	}
    for (blk=0; blk<4; blk++){
        refidx[blk] = (int)vlc_input->refidx[blk];
    }
    /*
    mv_fh0 = vlc_input->mv_fh0;
    mv_fv0 = vlc_input->mv_fv0;
    mv_bh0 = vlc_input->mv_bh0;
    mv_bv0 = vlc_input->mv_bv0;
    mv_f[0][0] = vlc_input->mv_fh0;
    mv_f[0][1] = vlc_input->mv_fv0;
    mv_f[1][0] = vlc_input->mv_fh1;
    mv_f[1][1] = vlc_input->mv_fv1;
    mv_f[2][0] = vlc_input->mv_fh2;
    mv_f[2][1] = vlc_input->mv_fv2;
    mv_f[3][0] = vlc_input->mv_fh3;
    mv_f[3][1] = vlc_input->mv_fv3;
    mv_b[0][0] = vlc_input->mv_bh0;
    mv_b[0][1] = vlc_input->mv_bv0;
    mv_b[1][0] = vlc_input->mv_bh1;
    mv_b[1][1] = vlc_input->mv_bv1;
    mv_b[2][0] = vlc_input->mv_bh2;
    mv_b[2][1] = vlc_input->mv_bv2;
    mv_b[3][0] = vlc_input->mv_bh3;
    mv_b[3][1] = vlc_input->mv_bv3;
    */

    //(update_type) 0:normal_process, 1:intra_mb, 2:not_coded
    //<MZ031211> add P_8x8
    /*
    if (update_only == 1){
    mv_h = mv_v = 0;
    refidx = -1;
    }else if (update_only == 2){
    mv_h = mv_v = 0;
    refidx = 0;
    }else{
    if (me_type == 1){
    mv_h = mv_fh0;
    mv_v = mv_fv0;
    refidx = 0;
    }else{
    mv_h = mv_bh0;
    mv_v = mv_bv0;
    refidx = 1;
    }
    }
    */
    if (update_only == 1){
        for (blk=0; blk<4; blk++){
            mv[blk][0] = mv[blk][1] = 0;
            //<MZ040107> adopt multi reference
            //			refidx[blk] = -1;
        }
        //<MZ040107> improve P_skip
        //	}else if (update_only == 2){
        //		for (blk=0; blk<4; blk++){
        //			mv[blk][0] = mv[blk][1] = 0;
        //<MZ040107> adopt multi reference
        //			refidx[blk] = 0;
        //		}
    }else{
        //<MZ040107> adopt multi reference
        /*
        for (blk=0; blk<4; blk++){
        if (me_type[blk] == 1){
        mv[blk][0] = mv_f[blk][0];
        mv[blk][1] = mv_f[blk][1];
        refidx[blk] = 0;
        }else{
        mv[blk][0] = mv_b[blk][0];
        mv[blk][1] = mv_b[blk][1];
        refidx[blk] = 1;
        }
        }
        */
        mv[0][0] = vlc_input->mv_fh0;
        mv[0][1] = vlc_input->mv_fv0;
        mv[1][0] = vlc_input->mv_fh1;
        mv[1][1] = vlc_input->mv_fv1;
        mv[2][0] = vlc_input->mv_fh2;
        mv[2][1] = vlc_input->mv_fv2;
        mv[3][0] = vlc_input->mv_fh3;
        mv[3][1] = vlc_input->mv_fv3;
    }

    len = 0;

    //<MZ031211> add P_8x8
    //	m4vac_pmv_calc_avc(&mv_h, &mv_v, &refidx, &mvp_h, &mvp_v,
    //		mb_posx, mb_posy, vp_enable, vp_mb_num, mb_sizex, 0);
    //<MZ040107> improve P_skip
    //	m4vac_pmv_calc_avc(mv, refidx, mvp,
    //		mb_posx, mb_posy, vp_enable, vp_mb_num, mb_sizex, mb_type);
    m4vac_pmv_calc_avc(mv, refidx, mvp,
        mb_posx, mb_posy, vp_enable, vp_mb_num, mb_sizex, mb_type,
        0, &mba_rf, &mbb_rf);

    //printf("VLC : .... MB(%d, %d)\tPMV = (%d, %d)\n", mb_posy, mb_posx, mvp[0][0], mvp[0][1]);

    if (update_only == 0){
        //<MZ031211> add P_8x8
        /*
        if (use_multi_ref){
        VLCLOG("  refidx = %d\n", refidx);
        if (refidx == 0){
        len += putbits(1, 1);
        }else{
        len += putbits(0, 1);
        }
        }
        }

        mvd_h = mv_h - mvp_h;
        mvd_v = mv_v - mvp_v;

        //printf("MV (%d, %d) = MVP (%d, %d) + MVD (%d, %d)\n",
        //		mv_h, mv_v, mvp_h, mvp_v, mvd_h, mvd_v);
        VLCLOG("  mvd = (%d, %d)\n", mvd_h, mvd_v);

        len += put_se(mvd_h);
        len += put_se(mvd_v);
        */
        //<MZ040218> add ref_idx_last
        //		if (use_multi_ref && (mb_type != 4)){		//not describe refidx at P_8x8ref0
        if ((ref_idx_last != 0) && (mb_type != 4)){		//not describe refidx at P_8x8ref0
            for (blk=0; blk<4; blk++){
                if ((mb_type == 0) && (blk == 1)) break;
                if ((mb_type == 1) && ((blk == 1) || (blk == 3))) continue;
                if ((mb_type == 2) && (blk == 2)) break;
                VLCLOG("  ref_idx = %d\n", refidx[blk]);
                if (refidx[blk] == 0){
                    len += putbits(1, 1);
                }else{
                    len += putbits(0, 1);
                }
            }
        }
        for (blk=0; blk<4; blk++){
            if ((mb_type == 0) && (blk == 1)) break;
            if ((mb_type == 1) && ((blk == 1) || (blk == 3))) continue;
            if ((mb_type == 2) && (blk == 2)) break;
            mvd[blk][0] = mv[blk][0] - mvp[blk][0];
            mvd[blk][1] = mv[blk][1] - mvp[blk][1];
            VLCLOG("  mv (%d, %d) = mvp (%d, %d) + mvd (%d, %d)\n",
                mv[blk][0], mv[blk][1], mvp[blk][0], mvp[blk][1], mvd[blk][0], mvd[blk][1]);
            len += put_se(mvd[blk][0]);
            len += put_se(mvd[blk][1]);
            //<MZ040324> exam to improve skip
            if (mb_type == 0){
                //printf("vlc pmv = (%d, %d)\n", mvp[blk][0], mvp[blk][1]);
                //printf("vlc  mv = (%d, %d)\n", mv[blk][0], mv[blk][1]);
            }
        }
    }

    return len;
}

/******************************************************************************
*
* Function Name	: m4vac_pmv_calc_avc
*
*****************************************************************************/

//     +---+---+
//     |[1]|[2]|
// +---+---+---+
// |[0]|[3]| <- current MB
// +---+---+

void m4vac_pmv_calc_avc(
    //<MZ031211> add P_8x8
    //int *mv_h,
    //int *mv_v,
    int mv[4][2],
    int refidx[4],
    //<MZ031211> add P_8x8
    //int *mvp_h,
    //int *mvp_v,
    int mvp[4][2],
    int mb_posx,
    int mb_posy,
    int vp_enable,
    int vp_mb_num,
    int mb_sizex,
    //<MZ031215> add P_8x8
    //int inter4v)
    //<MZ040107> improve P_skip
    //int mb_type)
    int mb_type,
    int judge_p_skip,
    int *jps_mba_rf,
    int *jps_mbb_rf)
{
    static long line_mv[VP4S_MAX_MB_WIDTH][4];		//25bit*2*45
    static short cur_mv[4][2];		//block0123*HV
    static short mba_mv[2][4];		//block13*HV(left)
    static short mbb_mv[2][4];		//block23*HV(up)
    static short mbc_mv[2][4];		//block23*HV(upright)
    static short mbd_mv[2];
    //<MZ040107> fix P_8x8
    //	short mbc_calc[2];
    short mbc_calc[4][2];
    static int line_rf[VP4S_MAX_MB_WIDTH][4];		//refIdx*4*45
    static int cur_rf[4];			//block0123 refIdx
    static int mba_rf[2];			//block13 refIdx(left)
    static int mbb_rf[2];			//block23 refIdx(up)
    static int mbc_rf[2];			//block23 refIdx(upright)
    static int mbd_rf;
    //<MZ040107> fix P_8x8
    //	int mbc_calrf;
    int mbc_calrf[4];
    int i;
    //<MZ040107> improve P_skip
    int jps_mba_mv[2];
    int jps_mbb_mv[2];
    int jps_mbc_mv[2];
    int jps_mbd_mv[2];
    int jps_mbc_calc[2];
    //int jps_mba_rf;
    //int jps_mbb_rf;
    int jps_mbc_rf;
    int jps_mbd_rf;
    int jps_mbc_calrf;

    //<MZ040107> improve P_skip
    if (judge_p_skip == 1){
        // MBA
        if ((mb_posx == 0) || (vp_enable && (vp_mb_num == 0))){		//MV1
            jps_mba_mv[0] = jps_mba_mv[1] = MV_INVALID;
            *jps_mba_rf = RF_INVALID;
        }else{
            jps_mba_mv[0] = cur_mv[1][0];
            jps_mba_mv[1] = cur_mv[1][1];
            *jps_mba_rf = cur_rf[1];
        }
        // MBD
        if ((mb_posy == 0) || (mb_posx == 0) || (vp_enable && (vp_mb_num < mb_sizex + 1))){	//MV4
            jps_mbd_mv[0] = jps_mbd_mv[1] = MV_INVALID;
            jps_mbd_rf = RF_INVALID;
        }else{
            jps_mbd_mv[0] = mbb_mv[1][0];
            jps_mbd_mv[1] = mbb_mv[1][1];
            jps_mbd_rf = mbb_rf[1];
        }
        // MBB
        if ((mb_posy == 0) || (vp_enable && (vp_mb_num < mb_sizex))){	//MV2
            jps_mbb_mv[0] = jps_mbb_mv[1] = MV_INVALID;
            *jps_mbb_rf = RF_INVALID;
        }else if (mb_posx == 0){
            jps_mbb_mv[0] = (short)((line_mv[0][0] & 0xffff0000) >> 16);
            jps_mbb_mv[1] = (short)(line_mv[0][0] & 0x0000ffff);
            *jps_mbb_rf = line_rf[0][0];
        }else{
            jps_mbb_mv[0] = mbc_mv[0][0];
            jps_mbb_mv[1] = mbc_mv[0][1];
            *jps_mbb_rf = mbc_rf[0];
        }
        // MBC
        if ((mb_posy == 0) || (mb_posx == mb_sizex - 1)
            || (vp_enable && (vp_mb_num < mb_sizex - 1))){		//MV3
                jps_mbc_mv[0] = jps_mbc_mv[1] = MV_INVALID;
                jps_mbc_rf = RF_INVALID;
        }else{
            jps_mbc_mv[0] = (short)((line_mv[mb_posx+1][0] & 0xffff0000) >> 16);
            jps_mbc_mv[1] = (short)(line_mv[mb_posx+1][0] & 0x0000ffff);
            jps_mbc_rf = line_rf[mb_posx+1][0];
        }
        // MBC_CALC
        jps_mbc_calc[0] = (jps_mbc_mv[0] == MV_INVALID)? jps_mbd_mv[0]: jps_mbc_mv[0];
        jps_mbc_calc[1] = (jps_mbc_mv[1] == MV_INVALID)? jps_mbd_mv[1]: jps_mbc_mv[1];
        jps_mbc_calrf = (jps_mbc_rf == RF_INVALID)? jps_mbd_rf: jps_mbc_rf;
        // calcurate mvp
        for (i=0; i<2; i++){
            mvp[0][i] = m4vac_pred_mv_avc(
                (short)jps_mba_mv[i], (short)jps_mbb_mv[i], (short)jps_mbc_calc[i],
                *jps_mba_rf, *jps_mbb_rf, jps_mbc_calrf, refidx[0]);
            mvp[1][i] = jps_mba_mv[i];
            mvp[2][i] = jps_mbb_mv[i];
        }
        // end of operation
        return;
    }

    if ((mb_posx == 0) || (vp_enable && (vp_mb_num == 0))){		//MV1
        mba_mv[0][0] = mba_mv[0][1]
        = mba_mv[1][0] = mba_mv[1][1] = MV_INVALID;
        mba_rf[0] = mba_rf[1] = RF_INVALID;
    }else{
        mba_mv[0][0] = cur_mv[1][0];
        mba_mv[0][1] = cur_mv[1][1];
        mba_mv[1][0] = cur_mv[3][0];
        mba_mv[1][1] = cur_mv[3][1];
        mba_rf[0] = cur_rf[1];
        mba_rf[1] = cur_rf[3];
    }
    if ((mb_posy == 0) || (mb_posx == 0) || (vp_enable && (vp_mb_num < mb_sizex + 1))){	//MV4
        mbd_mv[0] = mbd_mv[1] = MV_INVALID;
        mbd_rf = RF_INVALID;
    }else{
        mbd_mv[0] = mbb_mv[1][0];
        mbd_mv[1] = mbb_mv[1][1];
        mbd_rf = mbb_rf[1];
    }
    if ((mb_posy == 0) || (vp_enable && (vp_mb_num < mb_sizex))){	//MV2
        mbb_mv[0][0] = mbb_mv[0][1]
        = mbb_mv[1][0] = mbb_mv[1][1] = MV_INVALID;
        mbb_rf[0] = mbb_rf[1] = RF_INVALID;
    }else if (mb_posx == 0){
        mbb_mv[0][0] = (short)((line_mv[0][0] & 0xffff0000) >> 16);
        mbb_mv[0][1] = (short)(line_mv[0][0] & 0x0000ffff);
        mbb_mv[1][0] = (short)((line_mv[0][1] & 0xffff0000) >> 16);
        mbb_mv[1][1] = (short)(line_mv[0][1] & 0x0000ffff);
        mbb_rf[0] = line_rf[0][0];
        mbb_rf[1] = line_rf[0][1];
    }else{
        mbb_mv[0][0] = mbc_mv[0][0];
        mbb_mv[0][1] = mbc_mv[0][1];
        mbb_mv[1][0] = mbc_mv[1][0];
        mbb_mv[1][1] = mbc_mv[1][1];
        mbb_rf[0] = mbc_rf[0];
        mbb_rf[1] = mbc_rf[1];
    }
    if ((mb_posy == 0) || (mb_posx == mb_sizex - 1)
        || (vp_enable && (vp_mb_num < mb_sizex - 1))){		//MV3
            mbc_mv[0][0] = mbc_mv[0][1]
            = mbc_mv[1][0] = mbc_mv[1][1] = MV_INVALID;
            mbc_rf[0] = mbc_rf[1] = RF_INVALID;
    }else{
        mbc_mv[0][0] = (short)((line_mv[mb_posx+1][0] & 0xffff0000) >> 16);
        mbc_mv[0][1] = (short)(line_mv[mb_posx+1][0] & 0x0000ffff);
        mbc_mv[1][0] = (short)((line_mv[mb_posx+1][1] & 0xffff0000) >> 16);
        mbc_mv[1][1] = (short)(line_mv[mb_posx+1][1] & 0x0000ffff);
        mbc_rf[0] = line_rf[mb_posx+1][0];
        mbc_rf[1] = line_rf[mb_posx+1][1];
    }

    //<MZ031215> add P_8x8
    //<MZ040106> add P_16x8, P_8x16
    /*
    //	if (inter4v){
    if ((mb_type == 3) || (mb_type == 4)){		//P_8x8(ref0)
    for (i=0; i<4; i++){
    //<MZ031211> add P_8x8
    //			cur_mv[i][0] = mv_h[i];
    //			cur_mv[i][1] = mv_v[i];
    cur_mv[i][0] = mv[i][0];
    cur_mv[i][1] = mv[i][1];
    cur_rf[i] = refidx[i];
    }
    }else{										//P_16x16
    for (i=0; i<4; i++){
    //<MZ031211> add P_8x8
    //			cur_mv[i][0] = mv_h[0];
    //			cur_mv[i][1] = mv_v[0];
    cur_mv[i][0] = mv[0][0];
    cur_mv[i][1] = mv[0][1];
    cur_rf[i] = refidx[0];
    }
    }
    */
    for (i=0; i<4; i++){
        cur_mv[i][0] = mv[i][0];
        cur_mv[i][1] = mv[i][1];
        cur_rf[i] = refidx[i];
    }
    line_mv[mb_posx][0] = ((cur_mv[2][0] << 16) | (cur_mv[2][1] & 0xffff));
    line_mv[mb_posx][1] = ((cur_mv[3][0] << 16) | (cur_mv[3][1] & 0xffff));
    line_rf[mb_posx][0] = cur_rf[2];
    line_rf[mb_posx][1] = cur_rf[3];

    //<MZ040107> fix P_8x8
    /*
    mbc_calc[0] = (mbc_mv[0][0] == MV_INVALID)? mbd_mv[0]: mbc_mv[0][0];
    mbc_calc[1] = (mbc_mv[0][1] == MV_INVALID)? mbd_mv[1]: mbc_mv[0][1];
    mbc_calrf = (mbc_rf[0] == RF_INVALID)? mbd_rf: mbc_rf[0];
    */
    //MbPartIdx0
    if ((mb_type == 2) || (mb_type == 3) || (mb_type == 4)){		//P_8x16, P_8x8(ref0)
        mbc_calc[0][0] = (mbb_mv[1][0] == MV_INVALID)? mbd_mv[0]: mbb_mv[1][0];
        mbc_calc[0][1] = (mbb_mv[1][1] == MV_INVALID)? mbd_mv[1]: mbb_mv[1][1];
        mbc_calrf[0] = (mbb_rf[1] == RF_INVALID)? mbd_rf: mbb_rf[1];
    }else{															//P_16x16, P_16x8
        mbc_calc[0][0] = (mbc_mv[0][0] == MV_INVALID)? mbd_mv[0]: mbc_mv[0][0];
        mbc_calc[0][1] = (mbc_mv[0][1] == MV_INVALID)? mbd_mv[1]: mbc_mv[0][1];
        mbc_calrf[0] = (mbc_rf[0] == RF_INVALID)? mbd_rf: mbc_rf[0];
    }
    //MbPartIdx1
    if (mb_type == 1){												//P_16x8
        mbc_calc[1][0] = mba_mv[0][0];
        mbc_calc[1][1] = mba_mv[0][1];
        mbc_calrf[1] = mba_rf[0];
    }else{															//P_8x16, P_8x8(ref0)
        mbc_calc[1][0] = (mbc_mv[0][0] == MV_INVALID)? mbb_mv[0][0]: mbc_mv[0][0];
        mbc_calc[1][1] = (mbc_mv[0][1] == MV_INVALID)? mbb_mv[0][1]: mbc_mv[0][1];
        mbc_calrf[1] = (mbc_rf[0] == RF_INVALID)? mbb_rf[0]: mbc_rf[0];
    }
    //MbPartIdx2
    mbc_calc[2][0] = cur_mv[1][0];
    mbc_calc[2][1] = cur_mv[1][1];
    mbc_calrf[2] = cur_rf[1];
    //MbPartIdx3
    mbc_calc[3][0] = cur_mv[0][0];
    mbc_calc[3][1] = cur_mv[0][1];
    mbc_calrf[3] = cur_rf[0];

    //<MZ031211> add P_8x8
    /*
    if (mbb_mv[0][0] == MV_INVALID && mbc_calc[0] == MV_INVALID){
    mvp_h[0] = (mba_mv[0][0] == MV_INVALID)? 0: mba_mv[0][0];
    mvp_v[0] = (mba_mv[0][1] == MV_INVALID)? 0: mba_mv[0][1];

    }else if ((cur_rf[0] == mba_rf[0]) && (cur_rf[0] != mbb_rf[0]) && (cur_rf[0] != mbc_calrf)){
    mvp_h[0] = mba_mv[0][0];
    mvp_v[0] = mba_mv[0][1];

    }else if ((cur_rf[0] != mba_rf[0]) && (cur_rf[0] == mbb_rf[0]) && (cur_rf[0] != mbc_calrf)){
    mvp_h[0] = mbb_mv[0][0];
    mvp_v[0] = mbb_mv[0][1];

    }else if ((cur_rf[0] != mba_rf[0]) && (cur_rf[0] != mbb_rf[0]) && (cur_rf[0] == mbc_calrf)){
    mvp_h[0] = mbc_calc[0];
    mvp_v[0] = mbc_calc[1];

    }else{
    mvp_h[0] = m4vac_median_mv_avc(mba_mv[0][0], mbb_mv[0][0], mbc_calc[0]);
    mvp_v[0] = m4vac_median_mv_avc(mba_mv[0][1], mbb_mv[0][1], mbc_calc[1]);
    }
    */
    if ((mb_type == 3) || (mb_type == 4)){		//P_8x8(ref0)
        //MbPartIdx0
        for (i=0; i<2; i++){
            mvp[0][i] = m4vac_pred_mv_avc(
                mba_mv[0][i], mbb_mv[0][i], mbc_calc[0][i],
                mba_rf[0], mbb_rf[0], mbc_calrf[0], cur_rf[0]);
        }
        //MbPartIdx1
        for (i=0; i<2; i++){
            mvp[1][i] = m4vac_pred_mv_avc(
                cur_mv[0][i], mbb_mv[1][i], mbc_calc[1][i],
                cur_rf[0], mbb_rf[1], mbc_calrf[1], cur_rf[1]);
        }
        //MbPartIdx2
        for (i=0; i<2; i++){
            mvp[2][i] = m4vac_pred_mv_avc(
                mba_mv[1][i], cur_mv[0][i], mbc_calc[2][i],
                mba_rf[1], cur_rf[0], mbc_calrf[2], cur_rf[2]);
        }
        //MbPartIdx3
        for (i=0; i<2; i++){
            mvp[3][i] = m4vac_pred_mv_avc(
                cur_mv[2][i], cur_mv[1][i], mbc_calc[3][i],
                cur_rf[2], cur_rf[1], mbc_calrf[3], cur_rf[3]);
        }
        //<MZ040106> add P_16x8, P_8x16
    }else if (mb_type == 1){					//P_16x8
        //MbPartIdx0
        if (cur_rf[0] == mbb_rf[0]){
            for (i=0; i<2; i++){
                mvp[0][i] = mvp[1][i] = mbb_mv[0][i];
            }
        }else{
            for (i=0; i<2; i++){
                mvp[0][i] = mvp[1][i] = m4vac_pred_mv_avc(
                    mba_mv[0][i], mbb_mv[0][i], mbc_calc[0][i],
                    mba_rf[0], mbb_rf[0], mbc_calrf[0], cur_rf[0]);
            }
        }
        //MbPartIdx1
        if (cur_rf[2] == mba_rf[1]){
            for (i=0; i<2; i++){
                mvp[2][i] = mvp[3][i] = mba_mv[1][i];
            }
        }else{
            for (i=0; i<2; i++){
                mvp[2][i] = mvp[3][i] = m4vac_pred_mv_avc(
                    mba_mv[1][i], cur_mv[0][i], mbc_calc[1][i],
                    mba_rf[1], cur_rf[0], mbc_calrf[1], cur_rf[2]);
            }
        }
    }else if (mb_type == 2){					//P_8x16
        //MbPartIdx0
        if (cur_rf[0] == mba_rf[0]){
            for (i=0; i<2; i++){
                mvp[0][i] = mvp[2][i] = mba_mv[0][i];
            }
        }else{
            for (i=0; i<2; i++){
                mvp[0][i] = mvp[2][i] = m4vac_pred_mv_avc(
                    mba_mv[0][i], mbb_mv[0][i], mbc_calc[0][i],
                    mba_rf[0], mbb_rf[0], mbc_calrf[0], cur_rf[0]);
            }
        }
        //MbPartIdx1
        if (cur_rf[1] == mbc_calrf[1]){
            for (i=0; i<2; i++){
                mvp[1][i] = mvp[3][i] = mbc_calc[1][i];
            }
        }else{
            for (i=0; i<2; i++){
                mvp[1][i] = mvp[3][i] = m4vac_pred_mv_avc(
                    cur_mv[0][i], mbb_mv[1][i], mbc_calc[1][i],
                    cur_rf[0], mbb_rf[1], mbc_calrf[1], cur_rf[1]);
            }
        }
    }else{										//P_16x16
        for (i=0; i<2; i++){
            mvp[0][i] = m4vac_pred_mv_avc(
                mba_mv[0][i], mbb_mv[0][i], mbc_calc[0][i],
                mba_rf[0], mbb_rf[0], mbc_calrf[0], cur_rf[0]);
        }
    }

    return;
}

//<MZ031211>
/******************************************************************************
*
* Function Name	: m4vac_pred_mv_avc
*
*****************************************************************************/

short m4vac_pred_mv_avc(
    short mva,
    short mvb,
    short mvc,
    int refa,
    int refb,
    int refc,
    int ref)
{
    short mvp;

    if (mvb == MV_INVALID && mvc == MV_INVALID){
        mvp = (mva == MV_INVALID)? 0: mva;
    }else if ((ref == refa) && (ref != refb) && (ref != refc)){
        mvp = mva;
    }else if ((ref != refa) && (ref == refb) && (ref != refc)){
        mvp = mvb;
    }else if ((ref != refa) && (ref != refb) && (ref == refc)){
        mvp = mvc;
    }else{
        mvp = m4vac_median_mv_avc(mva, mvb, mvc);
    }

    return mvp;
}

/******************************************************************************
*
* Function Name	: m4vac_median_mv_avc
*
*****************************************************************************/

short m4vac_median_mv_avc(
    short mv1,
    short mv2,
    short mv3)
{
    short tmp;

    if (mv1 == MV_INVALID){
        mv1 = 0;
    }
    if (mv2 == MV_INVALID){
        mv2 = 0;
    }
    if (mv3 == MV_INVALID){
        mv3 = 0;
    }
    if (mv2 < mv1){
        tmp = mv1;
        mv1 = mv2;
        mv2 = tmp;
    }
    if (mv3 < mv2){
        tmp = mv2;
        mv2 = mv3;
        mv3 = tmp;
        if (mv2 < mv1){
            tmp = mv1;
            mv1 = mv2;
            mv2 = tmp;
        }
    }
    return mv2;
}

/******************************************************************************
*
* Function Name	: m4vac_residual
*
*****************************************************************************/

int m4vac_residual(
    long *coef_in,
    int intra16x16or8x8,
    int cbpy,
    int cbpc,
    int mb_posx,
    int mb_posy,
    //<MZ040209> fix multi slice encoding
    int mb_sizex,
    int vp_enable,
    int vp_mb_num,
    int is_field,
    int chroma_format_idc,
    int update_only)
{
    int len, i, j;
    static int total_left[4 + 4 + 4], total_line[VP4S_MAX_MB_WIDTH][4 + 2 + 2];	            //Y10,Y11,Y14,Y15 & U2,U3,V2,V3
    int total_top[4 + 2 + 2], total_coef[16 + 16 + 1 + 2], trailing_ones[16 + 16 + 1 + 2];  //4:2:2 16YAC + 16CAC + 1YDC + 2CDC = 35
    long coef_buf[512], *coef_buf_c;
    long dc_tmp[16], dc_buf[16];
    int nA, nB, nC;
    int intra16x16 = intra16x16or8x8 & 0x1;
    //residual()
    len = 0;
    coef_buf_c = coef_buf + 256;

    //load neighbor data
    if (mb_posx == 0){		//if not 0, total_left is set previously.
        for (i = 0; i < 12; i++){
            total_left[i] = TC_UNAVAIL;
        }
    }
    //<MZ040209> fix multi slice encoding
    else if (vp_enable && (vp_mb_num == 0)){		//if not 0, total_left is set previously.
        for (i = 0; i < 12; i++){
            total_left[i] = TC_UNAVAIL;
        }
    }
    if (mb_posy == 0){
        for (i = 0; i < 8; i++){
            total_top[i] = TC_UNAVAIL;
        }
        //<MZ040209> fix multi slice encoding
    }else if (vp_enable && (vp_mb_num < mb_sizex)){
        for (i = 0; i < 8; i++){
            total_top[i] = TC_UNAVAIL;
        }
    }else{
        for (i = 0; i < 8; i++){
            total_top[i] = total_line[mb_posx][i];
        }
    }

    //<MZ040108> add I_PCM
    if (update_only == 2){			// I_PCM
        for (i = 0; i < 32; i++){
            if ((chroma_format_idc < 2 && i < 24) || chroma_format_idc == 2) {
                total_coef[i] = 16;
                vlc_mbinfo.total_coef[i] = 16;
            }
            else {
                total_coef[i] = 0;
                vlc_mbinfo.total_coef[i] = 0;
            }
            vlc_mbinfo.trailing_ones[i] = 0;
        }
        vlc_mbinfo.total_coef[Y_DC_IDX] = vlc_mbinfo.total_coef[U_DC_IDX] = vlc_mbinfo.total_coef[V_DC_IDX] = 0;
        vlc_mbinfo.trailing_ones[Y_DC_IDX] = vlc_mbinfo.trailing_ones[U_DC_IDX] = vlc_mbinfo.trailing_ones[V_DC_IDX] = 0;
    }else

        if (update_only == 1){			// not_coded
            for (i = 0; i < 35; i++){
                total_coef[i] = 0;
                vlc_mbinfo.total_coef[i] = 0;
                vlc_mbinfo.trailing_ones[i] = 0;
            }
        }else{
#ifdef iVCP1_VLC_HM
            if(intra16x16or8x8 & 0x2) { //I8x8
                //Luma
                for(i = 0; i < 4; i++) {
                    m4vac_scan_8x8_cavlc(&coef_in[i*64], &coef_buf[i*64], is_field);
                }
                //Chroma
                if (chroma_format_idc == 2) {
                    for (i = 16; i < 32; i++){
                        m4vac_scan_4x4(&coef_in[i * 16], &coef_buf[i * 16], is_field);
                    }
                }
                else {
                    for (i = 16; i < 24; i++){
                        m4vac_scan_4x4(&coef_in[i * 16], &coef_buf[i * 16], is_field);
                    }
                }
            }else {
#endif
                if (chroma_format_idc == 2) {
                    for (i = 0; i < 32; i++){
                        m4vac_scan_4x4(&coef_in[i * 16], &coef_buf[i * 16], is_field);
                    }
                }
                else {
                    for (i = 0; i < 24; i++){
                        m4vac_scan_4x4(&coef_in[i * 16], &coef_buf[i * 16], is_field);
                    }
                }
#ifdef iVCP1_VLC_HM
            }
#endif

            //Luma DC
            if (intra16x16){
                VLCLOG("      [Luma DC]\n");
                for (i=0; i<4; i++){
                    for (j=0; j<4; j++){
                        dc_tmp[i*4 + j] = coef_in[((i/2)*2+(j/2)) * 64 + ((i%2)*2+(j%2)) * 16];
                    }
                }
                m4vac_scan_4x4(dc_tmp, dc_buf, is_field);

                nC = m4vac_vlc_nC(total_left[0], total_top[0]);
                len += m4vac_residual_block(dc_buf, 0, 16, nC, (int*)&vlc_mbinfo.total_coef[Y_DC_IDX], (int*)&vlc_mbinfo.trailing_ones[Y_DC_IDX]);
            }
            else{
                vlc_mbinfo.total_coef[Y_DC_IDX] = 0;
                vlc_mbinfo.trailing_ones[Y_DC_IDX] = 0;
            }

            //Luma AC
            for (i=0; i<4; i++){
                if (cbpy & (0x1 << i)){
                    VLCLOG("      [Luma AC Block %d]\n", i);
                    //Luma AC Blk0
                    if ((i == 0) || (i == 2)){
                        nA = total_left[i];
                    }else{
                        nA = total_coef[i*4 - 3];
                    }
                    if ((i == 0) || (i == 1)){
                        nB = total_top[i*2];
                    }else{
                        nB = total_coef[i*4 - 6];
                    }
                    nC = m4vac_vlc_nC(nA, nB);
                    len += m4vac_residual_block(coef_buf+i*64,    intra16x16, 16, nC, &total_coef[i*4], &trailing_ones[i*4]);
                    vlc_mbinfo.total_coef[i*4] = total_coef[i*4];
                    vlc_mbinfo.trailing_ones[i*4] = trailing_ones[i*4];

                    //Luma AC Blk1
                    nA = total_coef[i*4];
                    if ((i == 0) || (i == 1)){
                        nB = total_top[i*2 + 1];
                    }else{
                        nB = total_coef[i*4 - 5];
                    }
                    nC = m4vac_vlc_nC(nA, nB);
                    len += m4vac_residual_block(coef_buf+i*64+16, intra16x16, 16, nC, &total_coef[i*4 + 1], &trailing_ones[i*4 + 1]);
                    vlc_mbinfo.total_coef[i*4 + 1] = total_coef[i*4 + 1];
                    vlc_mbinfo.trailing_ones[i*4 + 1] = trailing_ones[i*4 + 1];

                    //Luma AC Blk2
                    if ((i == 0) || (i == 2)){
                        nA = total_left[i + 1];
                    }else{
                        nA = total_coef[i*4 - 1];
                    }
                    nB = total_coef[i*4];
                    nC = m4vac_vlc_nC(nA, nB);
                    len += m4vac_residual_block(coef_buf+i*64+32, intra16x16, 16, nC, &total_coef[i*4 + 2], &trailing_ones[i*4 + 2]);
                    vlc_mbinfo.total_coef[i*4 + 2] = total_coef[i*4 + 2];
                    vlc_mbinfo.trailing_ones[i*4 + 2] = trailing_ones[i*4 + 2];

                    //Luma AC Blk3
                    nA = total_coef[i*4 + 2];
                    nB = total_coef[i*4 + 1];
                    nC = m4vac_vlc_nC(nA, nB);
                    len += m4vac_residual_block(coef_buf+i*64+48, intra16x16, 16, nC, &total_coef[i*4 + 3], &trailing_ones[i*4 + 3]);
                    vlc_mbinfo.total_coef[i*4 + 3] = total_coef[i*4 + 3];
                    vlc_mbinfo.trailing_ones[i*4 + 3] = trailing_ones[i*4 + 3];

                }else{
                    for (j=0; j<4; j++){
                        total_coef[i*4 + j] = 0;
                        vlc_mbinfo.total_coef[i*4 + j] = 0;
                        vlc_mbinfo.trailing_ones[i*4 + j] = 0;
                    }
                }
            }

            //Chroma DC
            if (cbpc){
                if (chroma_format_idc == 2) {
                    for (i = 16; i < 32; i++){
                        dc_tmp[i - 16] = coef_in[i * 16];
                    }
                    VLCLOG("      [Chroma DC]\n");
                    for (i = 0; i < 2; i++){
                        m4vac_scan_2x4(dc_tmp + i * 8, dc_buf + i * 8);
                        len += m4vac_residual_block(dc_buf + i * 8, 0, 8, -2, (int*)&vlc_mbinfo.total_coef[U_DC_IDX + i], (int*)&vlc_mbinfo.trailing_ones[U_DC_IDX + i]);
                    }
                }
                else {
                    for (i = 16; i < 24; i++){
                        dc_buf[i - 16] = coef_in[i * 16];
                    }
                    VLCLOG("      [Chroma DC]\n");
                    for (i = 0; i < 2; i++){
                        len += m4vac_residual_block(dc_buf + i * 4, 0, 4, -1, (int*)&vlc_mbinfo.total_coef[U_DC_IDX + i], (int*)&vlc_mbinfo.trailing_ones[U_DC_IDX + i]);
                    }
                }
            }
            else{
                vlc_mbinfo.total_coef[U_DC_IDX] = vlc_mbinfo.total_coef[V_DC_IDX] = 0;
                vlc_mbinfo.trailing_ones[U_DC_IDX] = vlc_mbinfo.trailing_ones[V_DC_IDX] = 0;
            }

            //Chroma AC
            if (cbpc & 0x2){
                for (i=0; i<2; i++){
                    VLCLOG("      [Chroma AC Block %d]\n", i);

                    if (chroma_format_idc == 2) {
                        //Chroma AC Blk0
                        nC = m4vac_vlc_nC(total_left[4 + i * 4], total_top[4 + i * 2]);
                        len += m4vac_residual_block(coef_buf_c + i * 128, 1, 16, nC, &total_coef[16 + i * 8], &trailing_ones[16 + i * 8]);
                        vlc_mbinfo.total_coef[16 + i * 8] = total_coef[16 + i * 8];
                        vlc_mbinfo.trailing_ones[16 + i * 8] = trailing_ones[16 + i * 8];

                        //Chroma AC Blk1
                        nC = m4vac_vlc_nC(total_coef[16 + i * 8], total_top[4 + i * 2 + 1]);
                        len += m4vac_residual_block(coef_buf_c + i * 128 + 16, 1, 16, nC, &total_coef[16 + i * 8 + 1], &trailing_ones[16 + i * 8 + 1]);
                        vlc_mbinfo.total_coef[16 + i * 8 + 1] = total_coef[16 + i * 8 + 1];
                        vlc_mbinfo.trailing_ones[16 + i * 8 + 1] = trailing_ones[16 + i * 8 + 1];

                        //Chroma AC Blk2
                        nC = m4vac_vlc_nC(total_left[4 + i * 4 + 1], total_coef[16 + i * 8 + 0]);
                        len += m4vac_residual_block(coef_buf_c + i * 128 + 32, 1, 16, nC, &total_coef[16 + i * 8 + 2], &trailing_ones[16 + i * 8 + 2]);
                        vlc_mbinfo.total_coef[16 + i * 8 + 2] = total_coef[16 + i * 8 + 2];
                        vlc_mbinfo.trailing_ones[16 + i * 8 + 2] = trailing_ones[16 + i * 8 + 2];

                        //Chroma AC Blk3
                        nC = m4vac_vlc_nC(total_coef[16 + i * 8 + 2], total_coef[16 + i * 8 + 1]);
                        len += m4vac_residual_block(coef_buf_c + i * 128 + 48, 1, 16, nC, &total_coef[16 + i * 8 + 3], &trailing_ones[16 + i * 8 + 3]);
                        vlc_mbinfo.total_coef[16 + i * 8 + 3] = total_coef[16 + i * 8 + 3];
                        vlc_mbinfo.trailing_ones[16 + i * 8 + 3] = trailing_ones[16 + i * 8 + 3];

                        //Chroma AC Blk4
                        nC = m4vac_vlc_nC(total_left[4 + i * 4 + 2], total_coef[16 + i * 8 + 2]);
                        len += m4vac_residual_block(coef_buf_c + i * 128 + 64, 1, 16, nC, &total_coef[16 + i * 8 + 4], &trailing_ones[16 + i * 8 + 4]);
                        vlc_mbinfo.total_coef[16 + i * 8 + 4] = total_coef[16 + i * 8 + 4];
                        vlc_mbinfo.trailing_ones[16 + i * 8 + 4] = trailing_ones[16 + i * 8 + 4];

                        //Chroma AC Blk5
                        nC = m4vac_vlc_nC(total_coef[16 + i * 8 + 4], total_coef[16 + i * 8 + 3]);
                        len += m4vac_residual_block(coef_buf_c + i * 128 + 80, 1, 16, nC, &total_coef[16 + i * 8 + 5], &trailing_ones[16 + i * 8 + 5]);
                        vlc_mbinfo.total_coef[16 + i * 8 + 5] = total_coef[16 + i * 8 + 5];
                        vlc_mbinfo.trailing_ones[16 + i * 8 + 5] = trailing_ones[16 + i * 8 + 5];

                        //Chroma AC Blk6
                        nC = m4vac_vlc_nC(total_left[4 + i * 4 + 3], total_coef[16 + i * 8 + 4]);
                        len += m4vac_residual_block(coef_buf_c + i * 128 + 96, 1, 16, nC, &total_coef[16 + i * 8 + 6], &trailing_ones[16 + i * 8 + 6]);
                        vlc_mbinfo.total_coef[16 + i * 8 + 6] = total_coef[16 + i * 8 + 6];
                        vlc_mbinfo.trailing_ones[16 + i * 8 + 6] = trailing_ones[16 + i * 8 + 6];

                        //Chroma AC Blk7
                        nC = m4vac_vlc_nC(total_coef[16 + i * 8 + 6], total_coef[16 + i * 8 + 5]);
                        len += m4vac_residual_block(coef_buf_c + i * 128 + 112, 1, 16, nC, &total_coef[16 + i * 8 + 7], &trailing_ones[16 + i * 8 + 7]);
                        vlc_mbinfo.total_coef[16 + i * 8 + 7] = total_coef[16 + i * 8 + 7];
                        vlc_mbinfo.trailing_ones[16 + i * 8 + 7] = trailing_ones[16 + i * 8 + 7];
                    }
                    else {
                        //Chroma AC Blk0
                        nC = m4vac_vlc_nC(total_left[4 + i * 2], total_top[4 + i * 2]);
                        len += m4vac_residual_block(coef_buf_c + i * 64, 1, 16, nC, &total_coef[16 + i * 4], &trailing_ones[16 + i * 4]);
                        vlc_mbinfo.total_coef[16 + i * 4] = total_coef[16 + i * 4];
                        vlc_mbinfo.trailing_ones[16 + i * 4] = trailing_ones[16 + i * 4];

                        //Chroma AC Blk1
                        nC = m4vac_vlc_nC(total_coef[16 + i * 4], total_top[4 + i * 2 + 1]);
                        len += m4vac_residual_block(coef_buf_c + i * 64 + 16, 1, 16, nC, &total_coef[16 + i * 4 + 1], &trailing_ones[16 + i * 4 + 1]);
                        vlc_mbinfo.total_coef[16 + i * 4 + 1] = total_coef[16 + i * 4 + 1];
                        vlc_mbinfo.trailing_ones[16 + i * 4 + 1] = trailing_ones[16 + i * 4 + 1];

                        //Chroma AC Blk2
                        nC = m4vac_vlc_nC(total_left[4 + i * 2 + 1], total_coef[16 + i * 4]);
                        len += m4vac_residual_block(coef_buf_c + i * 64 + 32, 1, 16, nC, &total_coef[16 + i * 4 + 2], &trailing_ones[16 + i * 4 + 2]);
                        vlc_mbinfo.total_coef[16 + i * 4 + 2] = total_coef[16 + i * 4 + 2];
                        vlc_mbinfo.trailing_ones[16 + i * 4 + 2] = trailing_ones[16 + i * 4 + 2];

                        //Chroma AC Blk3
                        nC = m4vac_vlc_nC(total_coef[16 + i * 4 + 2], total_coef[16 + i * 4 + 1]);
                        len += m4vac_residual_block(coef_buf_c + i * 64 + 48, 1, 16, nC, &total_coef[16 + i * 4 + 3], &trailing_ones[16 + i * 4 + 3]);
                        vlc_mbinfo.total_coef[16 + i * 4 + 3] = total_coef[16 + i * 4 + 3];
                        vlc_mbinfo.trailing_ones[16 + i * 4 + 3] = trailing_ones[16 + i * 4 + 3];
                    }
                }

            }else{
                for (i=0; i<2; i++){
                    for (j=0; j<8; j++){
                        total_coef[16 + i*8 + j] = 0;
                        vlc_mbinfo.total_coef[16 + i*8 + j] = 0;
                        vlc_mbinfo.trailing_ones[16 + i*8 + j] = 0;
                    }
                }
            }
        }

        //save neighbor data
        if (chroma_format_idc == 2) {
            int left_idx[12] = { 5, 7, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31 };
            int top_idx[8] = { 10, 11, 14, 15, 22, 23, 30, 31 };
            for (i = 0; i < 12; i++){
                total_left[i] = total_coef[left_idx[i]];
            }
            for (i = 0; i < 8; i++){
                total_line[mb_posx][i] = total_coef[top_idx[i]];
            }
        }
        else {
            for (i = 0; i < 8; i++){
                total_left[i] = total_coef[5 + i / 2 * 8 - i / 4 * 4 - i / 6 * 4 + i % 2 * 2];
                total_line[mb_posx][i] = total_coef[i / 2 * 4 + 10 + i % 2];
            }
        }

        return len;
}

/******************************************************************************
*
* Function Name	: m4vac_residual_block
*
*****************************************************************************/

int m4vac_residual_block(
    long *coef_in,
    int start,
    int end,
    int nC,
    int *total_coef_out,
    int *trailing_ones_out)
{
    int len, i;
    int run, level, coef_cnt, max_coeff;
    int total_coeff, total_zeros, trailing_ones;
    int level_list[16], run_list[16];
    //int suffix_length, level_code, level_prefix, level_suffix;
    int suffix_length;                              //MZ121030
    long level_code, level_prefix, level_suffix;    //MZ121030
    int suffix_length_code;
    //	static int total_list[720 / 16 * 2];
    int level_large;    //MZ121030

    len = 0;
    run = 0;
    total_coeff = 0;
    total_zeros = 0;
    trailing_ones = 0;
    coef_cnt = start;
    coef_in += coef_cnt;
    max_coeff = end - start;

    //coeff_token
    while (coef_cnt < end){
        coef_cnt++;
        level = *coef_in++;

        if (level == 0){
            run++;
        }else{
            //			printf("(Run, Level) = (%d, %d)\n", run, level);
            level_list[total_coeff] = level;
            run_list[total_coeff] = run;
            total_coeff++;
            total_zeros += run;
            if (abs(level) == 1){
                if (trailing_ones < 3){
                    trailing_ones++;
                }
            }else{
                trailing_ones = 0;
            }
            run = 0;
        }
    }
    VLCLOG("      coeff_token: nC=%d, trailing_ones=%d, total_coeff=%d\n", nC, trailing_ones, total_coeff);

    len += put_coeff_token(trailing_ones, total_coeff, nC);
    if (total_coeff > 0){
        //trailing_ones_sign_flag
        for (i=0; i<trailing_ones; i++){
            level = level_list[total_coeff - 1 - i];

            VLCLOG("        level:%4d  ===  (TrailingOnesSignFlag) = (%d)\n", level, level == -1);
            len += putbits((level == -1), 1);
        }

        if ((total_coeff > 10) && (trailing_ones < 3)){
            suffix_length = 1;
        }else{
            suffix_length = 0;
        }

        //coeff_level
        for (; i<total_coeff; i++){
            level = level_list[total_coeff - 1 - i];
            if (level > 0){
                level_code = (level - 1) << 1;
            }else{
                level_code = -(level << 1) - 1;
            }
            if ((i == trailing_ones) && (trailing_ones < 3)){
                level_code -= 2;
            }

            suffix_length_code = suffix_length;
            level_large = 0;    //MZ121030

            if (suffix_length == 0){
                if (level_code <= 13){
                    level_prefix = level_code;
                    level_suffix = 0;

                    len += put_level_prefix(level_prefix);
                }else if (level_code <= 29){
                    level_prefix = 14;
                    level_suffix = level_code - 14;
                    suffix_length_code = 4;

                    len += put_level_prefix(level_prefix);
                    len += putbits(level_suffix, suffix_length_code);	//level_suffix
                    //MZ121024
                    //}else{  // max level_code = 4095 + 30 = 4125 > s12
                    //	level_prefix = 15;
                    //	level_suffix = level_code - 30;
                    //	suffix_length_code = 12;

                    //	len += put_level_prefix(level_prefix);
                    //	len += putbits(level_suffix, suffix_length_code);	//level_suffix
                    //}
                }else{
                    level_code -= 30;
                    level_large = 1;
                }

            }else{
                level_prefix = level_code >> suffix_length;
                if (level_prefix < 15){
                    level_suffix = level_code & ((1 << suffix_length) - 1);

                    len += put_level_prefix(level_prefix);
                    len += putbits(level_suffix, suffix_length_code);
                    //MZ121024
                    //}else{  // max level_code = 4095 + (15 << suffix_length) = 4125 @suffix_length=1 > s12
                    //	level_prefix = 15;
                    //	level_suffix = level_code - (15 << suffix_length);
                    //	suffix_length_code = 12;

                    //	len += put_level_prefix(level_prefix);
                    //	len += putbits(level_suffix, suffix_length_code);
                    //}
                }else{
                    level_code -= (15 << suffix_length);
                    level_large = 1;
                }
            }

            if (level_large == 1){
                if (level_code < (1 << 12)){    // s12
                    level_prefix = 15;
                    level_suffix = level_code;
                    suffix_length_code = 12;
                }else{
                    level_code += 4096;

                    if (level_code < (1 << 14)){
                        level_prefix = 16;
                        printf("Info(VLC): level_prefix = 16 at suffix_length = %d appears.\n", suffix_length);
                    }else if (level_code < (1 << 15)){
                        level_prefix = 17;
                        printf("Info(VLC): level_prefix = 17 at suffix_length = %d appears.\n", suffix_length);
                    }else if (level_code < (1 << 16)){
                        level_prefix = 18;
                        printf("Info(VLC): level_prefix = 18 at suffix_length = %d appears.\n", suffix_length);
                    }else if (level_code < (1 << 17)){  // s16
                        level_prefix = 19;
                        printf("Info(VLC): level_prefix = 19 at suffix_length = %d appears.\n", suffix_length);
                    }else if (level_code < (1 << 18)){
                        level_prefix = 20;
                        printf("Warning(VLC): level_prefix = 20 at suffix_length = %d appears.\n", suffix_length);
                    }else if (level_code < (1 << 19)){  // s18
                        level_prefix = 21;
                        printf("Warning(VLC): level_prefix = 21 at suffix_length = %d appears.\n", suffix_length);
                    }else{
                        // considering 10bit only
                        // levelCode >= 520222 (s19)
                        level_prefix = 22;
                        printf("Error(VLC): level_prefix > 21 at suffix_length = %d appears.\n", suffix_length);
                    }
                    level_suffix = level_code - (1 << (level_prefix - 3));
                    suffix_length_code = level_prefix - 3;
                }

                len += put_level_prefix(level_prefix);
                len += putbits(level_suffix, suffix_length_code);	//level_suffix
            }
            VLCLOG("        level:%4d =(%d)= (Prefix, Suffix) = (%ld, %ld)\n",
                level, suffix_length, level_prefix, level_suffix);
            //			VLCLOG("level_prefix :	%d\n", level_prefix);
            //			VLCLOG("suffix_length :	%d\n", suffix_length_code);
            //			VLCLOG("level :	%d, level_suffix : %d\n", level, level_suffix);

            if (suffix_length == 0){
                suffix_length++;
            }
            if ((abs(level) > (3 << (suffix_length - 1))) && (suffix_length < 6)){
                suffix_length++;
            }
        }

        //total_zeros
        if (total_coeff < max_coeff){
            VLCLOG("      total_zeros :	%d\n", total_zeros);

            if (end == 4){
                len += put_total_zeros_chromaDC(total_zeros, total_coeff);
            }
            else if (end == 8) {
                len += put_total_zeros_chromaDC422(total_zeros, total_coeff);
            }else{
                len += put_total_zeros(total_zeros, total_coeff);
            }
        }

        for (i=0; i<total_coeff-1; i++){
            if (total_zeros > 0){
                run = run_list[total_coeff - 1 - i];
                VLCLOG("        run :	%d\n", run);

                len += put_run_before(run, total_zeros);
                total_zeros -= run;
            }
        }
    }

    //if (total_coef_out != NULL){
    //	*total_coef_out = total_coeff;
    //}
    *total_coef_out = total_coeff;
    *trailing_ones_out = trailing_ones;

    return len;
}

/******************************************************************************
*
* Function Name	: put_coef_token
*
*****************************************************************************/

int put_coeff_token(
    int trailing_ones,
    int total_coeff,
    int nC)
{
    int value, length;

    if (nC == -2){
        value = value_coeff_token_list5[total_coeff][trailing_ones];
        length = length_coeff_token_list5[total_coeff][trailing_ones];
    }else if (nC == -1){
        value = value_coeff_token_list4[total_coeff][trailing_ones];
        length = length_coeff_token_list4[total_coeff][trailing_ones];
    }else if (nC < 2){
        value = value_coeff_token_list0[total_coeff][trailing_ones];
        length = length_coeff_token_list0[total_coeff][trailing_ones];
    }else if (nC < 4){
        value = value_coeff_token_list1[total_coeff][trailing_ones];
        length = length_coeff_token_list1[total_coeff][trailing_ones];
    }else if (nC < 8){
        value = value_coeff_token_list2[total_coeff][trailing_ones];
        length = length_coeff_token_list2[total_coeff][trailing_ones];
    }else{
        if (total_coeff == 0){
            value = 3;
        }else{
            value = ((total_coeff - 1) << 2) + trailing_ones;
        }
        length = 6;
    }

    return putbits(value, length);
}

/******************************************************************************
*
* Function Name	: put_level_prefix
*
*****************************************************************************/

int put_level_prefix(
    int level_prefix)
{
    return putbits(1, level_prefix + 1);
}

/******************************************************************************
*
* Function Name	: put_total_zeros
*
*****************************************************************************/

int put_total_zeros(
    int total_zeros,
    int total_coeff)
{
    int value, length;

    value = value_total_zeros_luma[total_coeff-1][total_zeros];
    length = length_total_zeros_luma[total_coeff-1][total_zeros];

    return putbits(value, length);
}

/******************************************************************************
*
* Function Name	: put_total_zeros_chromaDC
*
*****************************************************************************/

int put_total_zeros_chromaDC(
    int total_zeros,
    int total_coeff)
{
    int value, length;

    value = value_total_zeros_chroma[total_coeff-1][total_zeros];
    length = length_total_zeros_chroma[total_coeff-1][total_zeros];

    return putbits(value, length);
}


/******************************************************************************
*
* Function Name	: put_total_zeros_chromaDC
*
*****************************************************************************/

int put_total_zeros_chromaDC422(
    int total_zeros,
    int total_coeff)
{
    int value, length;

    value = value_total_zeros_chroma_dc422[total_coeff - 1][total_zeros];
    length = length_total_zeros_chroma_dc422[total_coeff - 1][total_zeros];

    return putbits(value, length);
}

/******************************************************************************
*
* Function Name	: put_total_zeros_chromaDC
*
*****************************************************************************/

int put_run_before(
    int run_before,
    int zeros_left)
{
    int value, length;

    if (zeros_left > 6){
        if (run_before < 7){
            value = 7 - run_before;
            length = 3;
        }else{
            value = 1;
            length = run_before - 3;
        }

    }else{
        value = value_run_before[zeros_left-1][run_before];
        length = length_run_before[zeros_left-1][run_before];
    }

    return putbits(value, length);
}

/******************************************************************************
*
* Function Name	: put_ue
*
*****************************************************************************/

int put_ue(
    int codenum)
{
    int tmp, length;
    long base, code;

    tmp = user_log2(codenum + 1);
    base = 1 << tmp;
    length = tmp * 2 + 1;
    code = base | (codenum + 1 - base);

    return putbits(code, length);
}

/******************************************************************************
*
* Function Name	: put_se
*
*****************************************************************************/

int put_se(
    int codenum)
{
    return put_ue((abs(codenum) << 1) - pos(codenum));
}

/******************************************************************************
*
* Function Name	: put_me
*
*****************************************************************************/

int put_me(
    int coded_block_pattern,
    int intra_mb,
    int chroma_format_idc
    )
{
    int codenum;

    if (intra_mb){
        if(chroma_format_idc == 0 || chroma_format_idc == 3) {
            codenum = cbp_intra_400_or_444[coded_block_pattern & 0xf];
        }else {
            codenum = cbp_intra[coded_block_pattern];
        }
    }else{
        if(chroma_format_idc == 0 || chroma_format_idc == 3) {
            codenum = cbp_inter_400_or_444[coded_block_pattern & 0xf];
        }else {
            codenum = cbp_inter[coded_block_pattern];
        }
    }
    return put_ue(codenum);
}
// == end miche.cpp

/****************************************************************************/
/*																			*/
/*	MPEG4core Cmodel														*/
/*																			*/
/*	Copyright (C) Renesas Technology Corp., 2003. All rights reserved.		*/
/*																			*/
/*	Version  1.0 : m4vac_vlc.c				2003/03/11 12:00				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*	Modification history													*/
/*	Ver.1.00	2003/03/11	start codes										*/
/*																			*/
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "m4vac_vlc.h"
#include "m4vac_common.h"
#include "m4vac_debug.h"
#include "m4vac_scan.h"			// 03/02/17 added
#include "m4vac_pred.h"			// 03/02/17 added
#include "m4vac_stream_rw.h"	// 04/01/23 added

m4vac_vlc_mbinfo vlc_mbinfo;
#ifdef iVCP1_VLC_HM
extern int slice_header_bits;
//unsigned int ems_buf_bit = 0;
extern unsigned int ems_buf_bit;
#endif

//for putbits()
int nbits_vop = 0;

static unsigned long bufp[3];
static int offset_bitp[3] = {0, 0, 0};
//<AK040123> STREAM_RW
//static int strm_buf_endian;
extern m4vac_strrw_input	strrw_input;
extern m4vac_strrw_output	strrw_output;

//#define DP2RAM_MAX (512/32)
//#define DP3RAM_MAX (2048/32)
//#define DPRAM_SIZE	(2048/32)		//for VPU3
#define DPRAM_SIZE	(4096/32)		//for VPU4

static int putpart = 0;
static int offset_long = 0;
static int offset_long2 = 0;
static int offset_long3 = 0;
static int dp3_close = 0;
static long dp_ram[DPRAM_SIZE];

static int hard_insert = 0;
static int inserted_bit = 0;
#ifdef iVCP1_VLC_HM
static int inserted_bit_buf = 0; 
#endif

/******************************************************************************
 *
 * Function Name	: m4vac_vlc
 *
 *****************************************************************************/

long m4vac_vlc(
               m4vac_vlc_input *vlc_input,
               m4vac_vlc_output *vlc_output)
{
    static int mb_quant;
    static int prev_mb_quant;
    static int vp_size;
    static int gob_number;
    static int mb_in_gob;
    //<MZ040728> change slice end flow
    static int slice_over, slice_over_mb_cnt;
    int len;
    int process_type, vop_type, dquant = 0, short_vh;
    int mb_posx, mb_posy, mb_sizex, mb_sizey, mb_num = 0;
    int vp_enable, vp_limit = 0, vp_mb_num = 0, vp_num, vp_close = 0;
    //<MZ040108> adopt slice encoding
    int vp_mbs_limit = 0;
    int dp_enable, dp2_close=0, coloc_not_coded_next = 0;
    long *coef_tmp;		// added 03/02/20
    int i;					// added 03/02/20
    static long vlc_coef_buf[512];	// added 03/02/20
    int h264;
    int annex_k = 0;
    //<MZ040910> add mb_out
    static long *dp2_addr;
    unsigned long lword;

#ifdef iVCP1_VLC_HM
    int slice_end = 0;
    static int slice_over_1d, slice_over_2d;
#ifdef iVCP1E2_ME_SPEC
    static int slice_over_3d;
#endif
    int mb_bits_org = 0;
#endif

    //<MZ041123> exam VLC perform
#ifdef EXAM_VLC_PERFORM
    static unsigned int mv_abs_sum, coef_abs_sum;
#endif

    //<IM040105>
    //	extern int test_slh_bits;

    process_type = vlc_input->process_type;
    short_vh = vlc_input->short_vh;
    //	strm_buf_endian = vlc_input->strm_buf_endian;
    h264 = vlc_input->h264;

    len = 0;
    inserted_bit = 0;
    if (process_type == BUF_RESET){
        //<AK040123> STREAM_RW
        //		stream_buf = (long*)((long)(vlc_output->stream_out) & 0xfffffffc);	//Static Global Variable 4byte boudary @cmodel 
        //<IM031225>															//hardware 16byte
        //		bufreset(vlc_output->stream_out);
        //		bufreset(vlc_output->stream_out, vlc_input->bit_offset);
        bufreset(vlc_input, vlc_output);
        nbits_vop = 0;
        m4vac_vlc_reset(vlc_output);
        hard_insert = vlc_input->emu_prev_byte;
        //<MZ040910> add mb_out
        dp2_addr = vlc_input->dp2_addr;

#ifdef iVCP1_VLC_HM
    }else if(process_type == VOP_FILLER) {
        hard_insert = vlc_input->emu_prev_byte;
        len = m4vac_filler(vlc_input->filler_size);
#endif
    }else if (process_type == BUF_FLUSH){
        bufflush();

    }else if (process_type == VOP_HEADER){
        if (h264){
            //do nothing
        }else if (short_vh){
            len = m4vac_vp_short_header(vlc_input, &mb_in_gob);
            //			printf("Picture Header includes %d bits.\n", len);
        }else{
            len = m4vac_vop_header(vlc_input);
            //			printf("VOP Header includes %d bits.\n", len);
            if (vlc_input->vop_type == 3){
                printf("Error vop_type : 3\n");
                vlc_output->err_code = 0xf0010000;
            }
        }
        //		printf("VOP Header length = %d\n", len);

        // mb_quant initialize
        mb_quant = prev_mb_quant = vlc_input->vop_quant;
        // video_packet initialize
        vp_size = 0;
        gob_number = 0;
        dp3_close = 0;
        m4vac_change_partition(1);
        vlc_input->jpeg_mode = 0;

    }else if (process_type == VOP_FOOTER){
        if (h264){
#ifndef iVCP1_VLC_HM
            len = m4vac_slice_trailing();
#endif
        }else if (short_vh){
            len = m4vac_vp_short_header_end(vlc_input->hext_enable);
        }else{
            len = m4vac_next_start_code();
        }

    }else if (process_type == RAW_OUTPUT){
        //<AK040123> new JPEG_MODE
        // SCAN
        coef_tmp = (long*)vlc_input->coef_in;
        vlc_input->jpeg_mode = 1;
        if (vlc_input->jpeg_yc){
            for (i=0; i<256; i++){
                vlc_coef_buf[i] = coef_tmp[i];
            }
        }else{
            for (i=0; i<128; i++){
                //				vlc_coef_buf[i] = coef_tmp[i+256];
                vlc_coef_buf[i] = coef_tmp[i];
            }
        }
        vlc_input->coef_in = (long*)vlc_coef_buf;

        m4vac_vlc_scan(vlc_input);

        len = m4vac_vlc_raw_output(vlc_input);

    }else if (process_type == MB_ENCODE){
        vop_type = vlc_input->vop_type;
        mb_posx = vlc_input->mb_posx;
        mb_posy = vlc_input->mb_posy;
        mb_sizex = vlc_input->mb_sizex;
        mb_sizey = vlc_input->mb_sizey;
        mb_num = (mb_posy * mb_sizex) + mb_posx;
        vp_num = gob_number;
        dp3_close = 0;

        vp_enable = vlc_input->vp_enable;
        vp_limit = vlc_input->vp_limit;
        //<MZ040108> adopt slice encoding
        vp_mbs_limit = vlc_input->vp_mbs_limit;
        //<MZ050307> unuse vp_enable at H.264
        if (h264){
            if ((vp_limit != 0) || (vp_mbs_limit != 0)){
                vp_enable = 1;
            }
#ifdef iVCP1_VLC_HM
            if(vlc_input->fms_flag) {
                if(mb_num == 0) {
                    vp_size += vlc_input->mb_target << 1;
                }
            }
#endif
        }
        vp_mb_num = vlc_input->vp_mb_num;
        dp_enable = vlc_input->dp_enable;
        coloc_not_coded_next = vlc_input->coloc_not_coded_next;
        annex_k = vlc_input->annex_k;

        if (vp_enable && (vp_mb_num == 0)){	// 03/02/24
            if (h264){
#ifdef iVCP1_VLC_HM
                if ((mb_posx == 0) && (mb_posy == 0)) {
#endif
                    bufreset(vlc_input, vlc_output);
                }
                nbits_vop = 0;		// VOP_BITS is output per slice
                //<MZ041108> clear number of inserted EPB
                vlc_output->nbits_ins = 0;
                //<MZ040728> change slice end flow
                slice_over_mb_cnt = 0;
                if (mb_num == 0){
                    slice_over = 0;
                    slice_over_1d = 0;
                    slice_over_2d = 0;
#ifdef iVCP1E2_ME_SPEC
                    slice_over_3d = 0;
#endif
                }
                //<MZ041022> fix output MB_ATTR
                vlc_output->notcoded_mbs = 0;
#ifdef EWS_LINUX
                if ((mb_posx == 0) && (mb_posy == 0)){
                    vlc_output->intra_mbs = 0;
                    vlc_output->inter_mbs = 0;
                }
#else
                vlc_output->intra_mbs = 0;
                vlc_output->inter_mbs = 0;
#endif
            }
            prev_mb_quant = mb_quant = vlc_input->mb_quant;
            dquant = mb_quant - prev_mb_quant;	// dquant should be 0
        }
        else {
            prev_mb_quant = mb_quant;
            mb_quant = vlc_input->mb_quant;
            dquant = mb_quant - prev_mb_quant;
            //<MZ040211> restrict range of dquant
            if (h264){
                //MZ121023
                if (vlc_input->ctrl_bit_depth_y == 0){
                    if (dquant < -26) dquant += 52;
                    if (dquant > 25) dquant -= 52;
                }
                else if (vlc_input->ctrl_bit_depth_y == 1) { //10b
                    if (dquant < -32) dquant += 64;
                    if (dquant > 31) dquant -= 64;
                }else { //12b
                    if (dquant < -38) dquant += 76;
                    if (dquant > 37) dquant -= 76;
                }
            }
        }

        // SCAN
        coef_tmp = (long*)vlc_input->coef_in;
        for (i=0; i<512; i++){
            vlc_coef_buf[i] = coef_tmp[i];
        }
        vlc_input->coef_in = (long*)vlc_coef_buf;
        m4vac_vlc_scan(vlc_input);
        for (i=0; i<6; i++){
            vlc_mbinfo.intra_dc[i] = 0;
        }
        for (i=0; i<4; i++){
            vlc_mbinfo.pmv_h[i] = 0;
            vlc_mbinfo.pmv_v[i] = 0;
        }
        if (vlc_input->vop_type == 0){
            vlc_input->ftop_ref = 0;
            vlc_input->fbot_ref = 0;
            vlc_input->btop_ref = 0;
            vlc_input->bbot_ref = 0;
        }
        if (vlc_input->me_type == 1){
            vlc_input->mv_bh0 = vlc_input->mv_fh0;
            vlc_input->mv_bv0 = vlc_input->mv_fv0;
            vlc_input->mv_bh1 = vlc_input->mv_fh1;
            vlc_input->mv_bv1 = vlc_input->mv_fv1;
        }else if (vlc_input->me_type == 2){
            vlc_input->mv_fh0 = vlc_input->mv_bh0;
            vlc_input->mv_fv0 = vlc_input->mv_bv0;
            vlc_input->mv_fh1 = vlc_input->mv_bh1;
            vlc_input->mv_fv1 = vlc_input->mv_bv1;
        }

        if (vp_enable && (vp_mb_num == 0)){
            //		printf("Video Packet Start: mb_posx=%d, mb_posy=%d, dp_enable=%d\n",
            //			mb_posx, mb_posy, dp_enable);
            //<AK040122> Annex.K
            if (short_vh && annex_k){
                //Annex.K includes slice following the picture start code.
                len += m4vac_vlc_slice_header_sh(vlc_input);
#ifndef iVCP1_VLC_HM
                vp_size = 0;
#endif
#ifdef VLC_DEBUG
                printf("Video Packet open at MB(%d,%d)\n", mb_posy, mb_posx);
#endif

            }else if (!((mb_posx == 0) && (mb_posy == 0))){
#ifdef VLC_DEBUG
                printf("Video Packet open at MB(%d,%d)\n", mb_posy, mb_posx);
#endif
#ifndef iVCP1_VLC_HM
                vp_size = 0;
#endif

                //<MZ040108> adopt slice encoding
                //				if (short_vh){
                if (h264){
                    // no operation
                }else if (short_vh){
                    len += m4vac_vlc_gob_header(vlc_input, gob_number);
                }else{
                    len += m4vac_vlc_vp_header(vlc_input);
                }
            }
            if (dp_enable){
                m4vac_change_partition(2);
                if (vop_type == 0){
                    len += putbits(0x6B001, 19);	//DC_MARKER
                }else if (vop_type == 1){
                    len += putbits(0x1F001, 17);	//MOTION_MARKER
                }
                m4vac_change_partition(1);
            }
        }

#ifdef VLC_DEBUG
        if (h264 == 0){
            printf("MB (%d, %d)", mb_posy, mb_posx);
        }
#endif

        if (h264 | short_vh){			//I|P-VOP only
            if (vop_type != 0 && vop_type != 1){
                printf("ERROR : vop_type %d is not supported for H.264/H.263, exit.\n", vop_type);
                exit(1);
            }
        }
        if (vop_type == 0){					//I-VOP
            if (h264){
                //<MZ040729> change slice end flow (fix)
                //				len += m4vac_vlc_mb_in_ipslice_avc(vlc_input, vlc_output, dquant);
                len += m4vac_vlc_mb_in_ipslice_avc(vlc_input, vlc_output, dquant, slice_over_mb_cnt);
            }else if (dp_enable){
                len += m4vac_vlc_mb_in_ivop_dp(vlc_input, vlc_output, dquant);
            }else{
                len += m4vac_vlc_mb_in_ivop(vlc_input, vlc_output, dquant);
            }
        }else if (vop_type == 1){			//P-VOP
            if (h264){
                len += m4vac_vlc_mb_in_ipslice_avc(vlc_input, vlc_output, dquant, slice_over_mb_cnt);
            }else if (dp_enable){
                len += m4vac_vlc_mb_in_pvop_dp(vlc_input, vlc_output, dquant);
            }else{
                len += m4vac_vlc_mb_in_pvop(vlc_input, vlc_output, dquant);
            }
        }else if (vop_type == 2){			//B-VOP
            len += m4vac_vlc_mb_in_bvop(vlc_input, vlc_output, dquant);
        }

        //<MZ041123> exam VLC perform
#ifdef EXAM_VLC_PERFORM
        coef_tmp = (long *)vlc_input->coef_in;
        if ((mb_posx == 0) && (mb_posy == 0)){
            mv_abs_sum = 0;
            coef_abs_sum = 0;
        }
        if (vlc_input->me_type == 0){
            // no operation
        }
        else if (vlc_input->me_type == 1){
            switch (vlc_input->mb_part){
                case 3:
                    //mv1
                    if (vlc_input->mv_fh1 < 0){
                        mv_abs_sum -= vlc_input->mv_fh1;
                    }
                    else{
                        mv_abs_sum += vlc_input->mv_fh1;
                    }
                    if (vlc_input->mv_fv1 < 0){
                        mv_abs_sum -= vlc_input->mv_fv1;
                    }
                    else{
                        mv_abs_sum += vlc_input->mv_fv1;
                    }
                    //mv2
                    if (vlc_input->mv_fh2 < 0){
                        mv_abs_sum -= vlc_input->mv_fh2;
                    }
                    else{
                        mv_abs_sum += vlc_input->mv_fh2;
                    }
                    if (vlc_input->mv_fv2 < 0){
                        mv_abs_sum -= vlc_input->mv_fv2;
                    }
                    else{
                        mv_abs_sum += vlc_input->mv_fv2;
                    }
                case 1:
                case 2:
                    //mv3
                    if (vlc_input->mv_fh3 < 0){
                        mv_abs_sum -= vlc_input->mv_fh3;
                    }
                    else{
                        mv_abs_sum += vlc_input->mv_fh3;
                    }
                    if (vlc_input->mv_fv3 < 0){
                        mv_abs_sum -= vlc_input->mv_fv3;
                    }
                    else{
                        mv_abs_sum += vlc_input->mv_fv3;
                    }
                case 0:
                    //mv0
                    if (vlc_input->mv_fh0 < 0){
                        mv_abs_sum -= vlc_input->mv_fh0;
                    }
                    else{
                        mv_abs_sum += vlc_input->mv_fh0;
                    }
                    if (vlc_input->mv_fv0 < 0){
                        mv_abs_sum -= vlc_input->mv_fv0;
                    }
                    else{
                        mv_abs_sum += vlc_input->mv_fv0;
                    }
                    break;
            }
        }
        else{
            printf("Warning : measure MV bits uncorrectly.\n");
        }
        for (i=0; i<384; i++){
            if (coef_tmp[i] < 0){
                coef_abs_sum -= coef_tmp[i];
            }
            else{
                coef_abs_sum += coef_tmp[i];
            }
        }
        if ((mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)){
            printf("mv_abs_sum   = %10d\tmv_bits   = %d\n", mv_abs_sum, vlc_output->mv_bits);
            printf("coef_abs_sum = %10d\tcoef_bits = %d\n", coef_abs_sum, vlc_output->tcoef_bits);
        }
#endif

#ifndef iVCP1_VLC_HM
        vp_size += len;
#endif
#if 0
        if (vp_enable){
            if(h264){
                if (slice_over){
                    slice_over_mb_cnt ++;
                }

                // judge slice close
                if ((mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)){
                    // case of picture end
                    vp_close = 1;
                }else{
                    dp2_close = m4vac_dp2_close(vop_type);
                    if (((mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1))
                        //<MZ040108> adopt slice encoding
                        //					|| (vp_size >= vp_limit)
                        //					|| ((vp_size >= vp_limit) && (!coloc_not_coded_next))
                        //<MZ040122> fix slice encoding
                        //					|| (((vp_size >= vp_limit) || (vp_mb_num >= vp_mbs_limit - 1)) && (!coloc_not_coded_next))
                        || ((((vp_size >= vp_limit) && (vp_limit != 0)) ||
                        ((vp_mb_num >= vp_mbs_limit - 1) && (vp_mbs_limit != 0))) && (!coloc_not_coded_next))
                        || (dp_enable && (dp3_close || dp2_close))){
                            vp_close = 1;
#ifdef VLC_DEBUG3
                            printf("Video Packet close at MB(%d,%d) (%d bits)\n",
                                mb_posy, mb_posx, vp_size);
                            printf("Cause: (bit_limit :%d) (mb_limit :%d) (dp2_limit :%d) (dp3_limit :%d)\n",
                                ((vp_size >= vp_limit) && (vp_limit != 0)),
                                ((vp_mb_num >= vp_mbs_limit - 1) && (vp_mbs_limit != 0)),
                                (dp_enable & dp2_close), (dp_enable & dp3_close) );
#endif
                            if (dp_enable && (vop_type != 2)){
                                if (dp3_close){
                                    //							dp3_close = 0;
                                    dpbits();
                                }else{
                                    dpbits();
                                    concatdp();
                                    dpbits();
                                }
                            }
                    }else{
                        vp_close = 0;
#ifdef VLC_DEBUG3
                        printf("Packet size is now %d bits. (limit %d)\n", vp_size, vp_limit);
#endif
                    }
                }
            }else{
                //<MZ050324> adjust MB info output
                //vp_close = 0;
                if (h264 && (mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)){
                    // case of picture end
                    vp_close = 1;
                }
                else{
                    vp_close = 0;
                }
            }
            if (vp_close){
                gob_number++;
            }
#endif

            if (vlc_input->field_pred){
                vlc_output->annex1++;
            }
            if (vlc_output->q_not_coded){
                mb_quant = prev_mb_quant;
            }

#ifdef iVCP1_VLC_HM

            vlc_output->nbits_mb = len + inserted_bit;

            if(vlc_input->fms_flag) {
                vp_size += slice_header_bits;
            }
            vp_size += vlc_output->nbits_mb;
            mb_bits_org = vlc_output->nbits_mb;

            if(vp_enable) {
                if(h264){
#ifdef iVCP1E2_ME_SPEC
                    slice_over_3d = slice_over_2d;
#endif
                    slice_over_2d = slice_over_1d;
                    slice_over_1d = slice_over;
                    //Slice division by MBs
                    if ((vp_mb_num >= vp_mbs_limit - 5) && (vp_mbs_limit > 4)){
                        // limit over ---> count start
                        slice_over = 1;
                        vp_size = 0;
                    }else {
                        slice_over = 0;
                    }

                    if (slice_over || slice_over_mb_cnt){
                        slice_over_mb_cnt ++;
                    }

                    // judge slice close
                    if ((mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)){
                        // case of picture end
                        vp_close = 1;
                        slice_end = 1;
                    }else if (   (vp_mbs_limit && slice_over_mb_cnt == 5 ) 
#ifndef iVCP1E2_ME_SPEC
                              || (vp_mbs_limit == 0 && slice_over_2d == 1)
#else
                              || (vp_mbs_limit == 0 && slice_over_3d == 1)
#endif
                             )
                    {
                        // case of slice end
                        vp_close = 0;
                        slice_end = 1;
                    }else{
                        vp_close = 0;
                    }
                }
            }else {
                if (h264 && (mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)){
                    // case of picture end
                    vp_close = 1;
                    slice_end = 1;
                }else{
                    vp_close = 0;
                }
            }

            if(h264 && slice_end) {
#ifdef iVCP1E_HM_SPEC
                if(vlc_output->not_coded && vlc_output->mb_skip_run != 0 && vp_limit != 0) {
                    VLCLOG("mb_skip_run: %ld\n", vlc_output->mb_skip_run);
                    len += put_ue(vlc_output->mb_skip_run);
                }
#endif
                len += m4vac_slice_trailing();
            }

            vlc_output->nbits_mb = len + inserted_bit;
            if(slice_end) {
                vlc_output->nbits_mb += inserted_bit_buf;
                vp_size += (vlc_output->nbits_mb - mb_bits_org);
            }

            //Slice division by bits
            if(vp_enable) {
                if(h264 && vp_limit != 0 && vp_mbs_limit == 0){
                    if (vp_size >= vp_limit){
                        slice_over = 1;
                        vp_size = 0;
                    }else {
                        slice_over = 0;
                    }
                }
            }

            vlc_output->slice_end = slice_end;

            if (slice_end){
                gob_number++;
            }
#else
            vlc_output->nbits_mb = len;
#endif
            vlc_output->vp_close = vp_close;
            //<MZ040108> adopt slice encoding
            vlc_output->vp_mb_num = vp_mb_num;
            vlc_output->npackets_vop = vp_num;
        }
        //<AK040329> hard insert EPB
        //<MZ040721> separate insert EPB
#ifdef iVCP1_VLC_HM
        nbits_vop += len + inserted_bit;
        if(slice_end) {
            nbits_vop += inserted_bit_buf;
            inserted_bit_buf = 0;
        }
#else
        nbits_vop += len;
#endif
        //<MZ041021> output error code
        if ((vlc_output->err_code == 0) && (nbits_vop > 0x00ffffff)){
            vlc_output->err_code = ERRE_VOP_SIZE | (vlc_input->mb_posy << 8) | vlc_input->mb_posx;
            //nbits_vop = 0x00ffffff;	// clip at the case of RTL
        }
#ifndef iVCP1_VLC_HM
        vlc_output->nbits_ins += inserted_bit;
#endif
        vlc_output->nbits_vop = nbits_vop;

#ifdef iVCP1_VLC_DBG
        if (process_type == MB_ENCODE){
            if(vlc_input->fms_flag)
                printf("VLC MB(%d) sh_bits %d mb_bit %d slc_bits %d\n", mb_num+3, slice_header_bits, len, vp_size);
            else
                printf("VLC MB(%d) sh_bits %d mb_bit %d slc_bits %d\n", mb_num+3, 0, len, vp_size);
        }
#endif

        //<MZ041129> evaluate force skip
#ifdef EXAM_FORCE_SKIP
        if (process_type == MB_ENCODE){
            //mb_pos (single slice only)
            printf("MB%d\t", vlc_output->vp_mb_num);
            //mb_type
            printf("intra= %d ", (vlc_input->me_type == 0));
            //SAD, MXD
            printf("SAD= %5d MXD= %3d ", vlc_input->sad, vlc_input->mxd);
            //CBP
            printf("CBP= %d\n", (vlc_mbinfo.cbp != 0));
        }
#endif

        //<MZ040910> add mb_out
        if (vlc_input->mb_out){
            if (process_type == MB_ENCODE){
                if (h264){
                    lword =
                        (vlc_output->vp_close << 31) |
                        (0 << 27) |		// conceal_busy : dec only
                        (vlc_output->vp_mb_num << 16) |
                        (vlc_input->mb_posy << 8) |
                        (vlc_input->mb_posx);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        (vlc_input->intra_chroma_type << 24) |
                        (vlc_mbinfo.cbp << 16) |
                        (0 << 8) |		// sub_mb_type : dec only
                        ((vlc_input->intra_pcm & (vlc_input->me_type == 0)) << 7) |
                        (((vlc_input->me_type == 0) && (vlc_input->mb_part == 0)) << 6) |
                        (((vlc_input->me_type != 0) * vlc_input->mb_part) << 4) |
                        (vlc_input->force_skip << 2) |
                        (vlc_output->not_coded << 1) |	// skip
                        (vlc_input->me_type == 0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword = (vlc_input->mb_quant << 24) | vlc_output->nbits_mb;
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        ((vlc_input->mxd & 0x000000ff) << 24) |
                        ((vlc_input->mad & 0x000000ff) << 16) |
                        (vlc_input->sad & 0x0000ffff);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);

                    lword =
                        (vlc_input->refidx[0] << 27) |
                        ((vlc_input->mv_fh0 & 0x00003fff) << 13) |
                        ((vlc_input->mv_fv0 & 0x00001fff) <<  0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        (vlc_input->refidx[1] << 27) |
                        ((vlc_input->mv_fh1 & 0x00003fff) << 13) |
                        ((vlc_input->mv_fv1 & 0x00001fff) <<  0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        (vlc_input->refidx[2] << 27) |
                        ((vlc_input->mv_fh2 & 0x00003fff) << 13) |
                        ((vlc_input->mv_fv2 & 0x00001fff) <<  0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        (vlc_input->refidx[3] << 27) |
                        ((vlc_input->mv_fh3 & 0x00003fff) << 13) |
                        ((vlc_input->mv_fv3 & 0x00001fff) <<  0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);

                    lword =
                        ((vlc_input->intra_luma_type[ 0] & 0x0000000f) << 28) |
                        ((vlc_input->intra_luma_type[ 1] & 0x0000000f) << 24) |
                        ((vlc_input->intra_luma_type[ 2] & 0x0000000f) << 20) |
                        ((vlc_input->intra_luma_type[ 3] & 0x0000000f) << 16) |
                        ((vlc_input->intra_luma_type[ 4] & 0x0000000f) << 12) |
                        ((vlc_input->intra_luma_type[ 5] & 0x0000000f) <<  8) |
                        ((vlc_input->intra_luma_type[ 6] & 0x0000000f) <<  4) |
                        ((vlc_input->intra_luma_type[ 7] & 0x0000000f) <<  0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        ((vlc_input->intra_luma_type[ 8] & 0x0000000f) << 28) |
                        ((vlc_input->intra_luma_type[ 9] & 0x0000000f) << 24) |
                        ((vlc_input->intra_luma_type[10] & 0x0000000f) << 20) |
                        ((vlc_input->intra_luma_type[11] & 0x0000000f) << 16) |
                        ((vlc_input->intra_luma_type[12] & 0x0000000f) << 12) |
                        ((vlc_input->intra_luma_type[13] & 0x0000000f) <<  8) |
                        ((vlc_input->intra_luma_type[14] & 0x0000000f) <<  4) |
                        ((vlc_input->intra_luma_type[15] & 0x0000000f) <<  0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        (0 << 31) |
                        ((vlc_mbinfo.trailing_ones[Y_DC_IDX] & 0x00000003) << 29) |
                        ((vlc_mbinfo.total_coef[Y_DC_IDX] & 0x0000001f) << 24) |
                        ((vlc_mbinfo.trailing_ones[ 0] & 0x00000003) << 22) |
                        ((vlc_mbinfo.total_coef[ 0]    & 0x0000001f) << 17) |
                        ((vlc_mbinfo.trailing_ones[ 1] & 0x00000003) << 15) |
                        ((vlc_mbinfo.total_coef[ 1]    & 0x0000001f) << 10) |
                        ((vlc_mbinfo.trailing_ones[ 2] & 0x00000003) <<  8) |
                        ((vlc_mbinfo.total_coef[ 2]    & 0x0000001f) <<  3) |
                        ((vlc_mbinfo.trailing_ones[ 3] & 0x00000003) <<  1) |
                        ((vlc_mbinfo.total_coef[ 3]    & 0x00000010) >>  4);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        ((vlc_mbinfo.total_coef[ 3]    & 0x0000000f) << 28) |
                        ((vlc_mbinfo.trailing_ones[ 4] & 0x00000003) << 26) |
                        ((vlc_mbinfo.total_coef[ 4]    & 0x0000001f) << 21) |
                        ((vlc_mbinfo.trailing_ones[ 5] & 0x00000003) << 19) |
                        ((vlc_mbinfo.total_coef[ 5]    & 0x0000001f) << 14) |
                        ((vlc_mbinfo.trailing_ones[ 6] & 0x00000003) << 12) |
                        ((vlc_mbinfo.total_coef[ 6]    & 0x0000001f) <<  7) |
                        ((vlc_mbinfo.trailing_ones[ 7] & 0x00000003) <<  5) |
                        ((vlc_mbinfo.total_coef[ 7]    & 0x0000001f) <<  0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);

                    lword =
                        (0 << 31) |
                        ((vlc_mbinfo.trailing_ones[ 8] & 0x00000003) << 29) |
                        ((vlc_mbinfo.total_coef[ 8]    & 0x0000001f) << 24) |
                        ((vlc_mbinfo.trailing_ones[ 9] & 0x00000003) << 22) |
                        ((vlc_mbinfo.total_coef[ 9]    & 0x0000001f) << 17) |
                        ((vlc_mbinfo.trailing_ones[10] & 0x00000003) << 15) |
                        ((vlc_mbinfo.total_coef[10]    & 0x0000001f) << 10) |
                        ((vlc_mbinfo.trailing_ones[11] & 0x00000003) <<  8) |
                        ((vlc_mbinfo.total_coef[11]    & 0x0000001f) <<  3) |
                        ((vlc_mbinfo.trailing_ones[12] & 0x00000003) <<  1) |
                        ((vlc_mbinfo.total_coef[12]    & 0x00000010) >>  4);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        ((vlc_mbinfo.total_coef[12]    & 0x0000000f) << 28) |
                        ((vlc_mbinfo.trailing_ones[13] & 0x00000003) << 26) |
                        ((vlc_mbinfo.total_coef[13]    & 0x0000001f) << 21) |
                        ((vlc_mbinfo.trailing_ones[14] & 0x00000003) << 19) |
                        ((vlc_mbinfo.total_coef[14]    & 0x0000001f) << 14) |
                        ((vlc_mbinfo.trailing_ones[15] & 0x00000003) << 12) |
                        ((vlc_mbinfo.total_coef[15]    & 0x0000001f) <<  7) |
                        ((vlc_mbinfo.trailing_ones[U_DC_IDX] & 0x00000003) << 5) |
                        ((vlc_mbinfo.total_coef[U_DC_IDX] & 0x0000001f) << 0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        (0 << 31) |
                        ((vlc_mbinfo.trailing_ones[16] & 0x00000003) << 29) |
                        ((vlc_mbinfo.total_coef[16]    & 0x0000001f) << 24) |
                        ((vlc_mbinfo.trailing_ones[17] & 0x00000003) << 22) |
                        ((vlc_mbinfo.total_coef[17]    & 0x0000001f) << 17) |
                        ((vlc_mbinfo.trailing_ones[18] & 0x00000003) << 15) |
                        ((vlc_mbinfo.total_coef[18]    & 0x0000001f) << 10) |
                        ((vlc_mbinfo.trailing_ones[19] & 0x00000003) <<  8) |
                        ((vlc_mbinfo.total_coef[19]    & 0x0000001f) <<  3) |
                        ((vlc_mbinfo.trailing_ones[V_DC_IDX] & 0x00000003) << 1) |
                        ((vlc_mbinfo.total_coef[V_DC_IDX] & 0x00000010) >> 4);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =
                        ((vlc_mbinfo.total_coef[20]    & 0x0000000f) << 28) |
                        ((vlc_mbinfo.trailing_ones[20] & 0x00000003) << 26) |
                        ((vlc_mbinfo.total_coef[20]    & 0x0000001f) << 21) |
                        ((vlc_mbinfo.trailing_ones[21] & 0x00000003) << 19) |
                        ((vlc_mbinfo.total_coef[21]    & 0x0000001f) << 14) |
                        ((vlc_mbinfo.trailing_ones[22] & 0x00000003) << 12) |
                        ((vlc_mbinfo.total_coef[22]    & 0x0000001f) <<  7) |
                        ((vlc_mbinfo.trailing_ones[23] & 0x00000003) <<  5) |
                        ((vlc_mbinfo.total_coef[23]    & 0x0000001f) <<  0);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);

                }else{
                    //<AK040924> VLE mb_info out
                    if (vlc_mbinfo.mb_type == 3 || vlc_mbinfo.mb_type == 4){
                        vlc_mbinfo.mv_h[0] = 0;
                        vlc_mbinfo.mv_h[1] = 0;
                        vlc_mbinfo.mv_h[2] = 0;
                        vlc_mbinfo.mv_h[3] = 0;
                        vlc_mbinfo.mv_v[0] = 0;
                        vlc_mbinfo.mv_v[1] = 0;
                        vlc_mbinfo.mv_v[2] = 0;
                        vlc_mbinfo.mv_v[3] = 0;
                    }else{
                        vlc_mbinfo.mv_h[0] = vlc_input->mv_fh0;
                        vlc_mbinfo.mv_h[1] = vlc_input->mv_fh1;
                        vlc_mbinfo.mv_h[2] = vlc_input->mv_bh0;
                        vlc_mbinfo.mv_h[3] = vlc_input->mv_bh1;
                        vlc_mbinfo.mv_v[0] = vlc_input->mv_fv0;
                        vlc_mbinfo.mv_v[1] = vlc_input->mv_fv1;
                        vlc_mbinfo.mv_v[2] = vlc_input->mv_bv0;
                        vlc_mbinfo.mv_v[3] = vlc_input->mv_bv1;
                    }

                    if (vlc_input->vop_type == 0 || vlc_input->interlaced == 0 || vlc_input->field_pred == 0){
                        vlc_mbinfo.ftop_ref = 0;
                        vlc_mbinfo.fbot_ref = 0;
                        vlc_mbinfo.btop_ref = 0;
                        vlc_mbinfo.bbot_ref = 0;
                    }else if (vlc_input->vop_type == 1){
                        vlc_mbinfo.ftop_ref = vlc_input->ftop_ref;
                        vlc_mbinfo.fbot_ref = vlc_input->fbot_ref;
                        vlc_mbinfo.btop_ref = 0;
                        vlc_mbinfo.bbot_ref = 0;
                    }else{
                        vlc_mbinfo.ftop_ref = vlc_input->ftop_ref;
                        vlc_mbinfo.fbot_ref = vlc_input->fbot_ref;
                        vlc_mbinfo.btop_ref = vlc_input->btop_ref;
                        vlc_mbinfo.bbot_ref = vlc_input->bbot_ref;
                    }
                    //<AK050422> update for dp_close, cause_close
                    vlc_mbinfo.cause_close = (short_vh & !annex_k)? 0:
                        (((vp_limit != 0) && (vp_size >= vp_limit) && (!coloc_not_coded_next)) << 2) |
                        (((vp_mbs_limit != 0) && (vp_mb_num >= vp_mbs_limit - 1) && (!coloc_not_coded_next)) << 1) |
                        ((dp2_close | dp3_close) << 0);

                    lword =		//vle_mb_info00
                        (1 << 31) |
                        (vlc_input->vop_type << 28) |
                        ((mb_num & 0xfff) << 16) |
                        (vlc_input->mb_posy << 8) |
                        (vlc_input->mb_posx);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info01
                        (vlc_mbinfo.cbp << 24) |
                        (vlc_output->not_coded << 23) |
                        (vlc_mbinfo.mb_type << 20) |
                        (vlc_input->coloc_not_coded << 19) |
                        (vlc_output->q_not_coded << 18) |
                        ((dquant != 0) << 17) |
                        ((vlc_input->ac_pred & 0x1) << 16) |
                        (vlc_input->field_dct << 13) |
                        (vlc_input->field_pred << 12) |
                        (vlc_mbinfo.ftop_ref << 11) |
                        (vlc_mbinfo.fbot_ref << 10) |
                        (vlc_mbinfo.btop_ref << 9) |
                        (vlc_mbinfo.bbot_ref << 8) |
                        (vlc_input->mb_quant);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info02
                        (vlc_output->vp_mb_num << 16) |
                        (vlc_input->me_type << 12) |
                        (vlc_output->npackets_vop & 0xfff);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info03
                        (offset_long2 << 24) |
                        (vlc_output->nbits_mb);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);

                    lword =		//vle_mb_info04
                        (offset_long3 << 24) |
                        (vlc_output->nbits_vop);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info05
                        ((vlc_input->vp_limit >> 16) << 28) |
                        (vp_close << 27) | (vlc_mbinfo.cause_close << 24) | (vp_size);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info06
                        (((vlc_input->vp_limit >> 8) & 0xff) << 24) |
                        (vlc_input->vop_minbit);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info07
                        (vlc_input->noscan << 27) |
                        (vlc_input->pred_scan_mode << 24) |
                        (vlc_input->pred_mb_direc << 16) |
                        (vlc_input->coef_number[0] << 8) |
                        (vlc_input->coef_number[1]);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);

                    lword =		//vle_mb_info08
                        ((vlc_input->coef_number[2]) << 24) |
                        ((vlc_input->coef_number[3]) << 16) |
                        ((vlc_input->coef_number[4]) << 8) |
                        (vlc_input->coef_number[5]);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info09
                        ((0x0fff & vlc_mbinfo.intra_dc[0]) << 16) |
                        ((0x0fff & vlc_mbinfo.intra_dc[1]));
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info10
                        ((0x0fff & vlc_mbinfo.intra_dc[2]) << 16) |
                        ((0x0fff & vlc_mbinfo.intra_dc[3]));
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info11
                        ((0x0fff & vlc_mbinfo.intra_dc[4]) << 16) |
                        ((0x0fff & vlc_mbinfo.intra_dc[5]));
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);

                    lword =		//vle_mb_info12
                        (((vlc_mbinfo.mv_h[0] >> 1) & 0xff) << 24) |
                        (((vlc_mbinfo.mv_v[0] >> 1) & 0xff) << 16) |
                        (((vlc_mbinfo.mv_h[1] >> 1) & 0xff) << 8) |
                        ((vlc_mbinfo.mv_v[1] >> 1) & 0xff);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info13
                        (((vlc_mbinfo.mv_h[2] >> 1) & 0xff) << 24) |
                        (((vlc_mbinfo.mv_v[2] >> 1) & 0xff) << 16) |
                        (((vlc_mbinfo.mv_h[3] >> 1) & 0xff) << 8) |
                        ((vlc_mbinfo.mv_v[3] >> 1) & 0xff);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info14
                        ((vlc_mbinfo.pmv_h[0] & 0xff) << 24) |
                        ((vlc_mbinfo.pmv_v[0] & 0xff) << 16) |
                        ((vlc_mbinfo.pmv_h[1] & 0xff) << 8) |
                        (vlc_mbinfo.pmv_v[1] & 0xff);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                    lword =		//vle_mb_info15
                        ((vlc_mbinfo.pmv_h[2] & 0xff) << 24) |
                        ((vlc_mbinfo.pmv_v[2] & 0xff) << 16) |
                        ((vlc_mbinfo.pmv_h[3] & 0xff) << 8) |
                        (vlc_mbinfo.pmv_v[3] & 0xff);
                    *dp2_addr++ = m4vac_vlc_swap_dp(lword, vlc_input->dp_ram_endian);
                }
            }
        }

        return 0;
    }

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_reset
 *
 *****************************************************************************/

int m4vac_vlc_reset(
                    m4vac_vlc_output *vlc_output)
{
    int i;

    vlc_output->not_coded = 0;
    vlc_output->q_not_coded = 0;
    vlc_output->nbits_mb = 0;
    vlc_output->nbits_vop = 0;
    //<MZ040721> separate insert EPB
    vlc_output->nbits_ins = 0;
    vlc_output->npackets_vop = 0;
    vlc_output->vp_close = 0;
    //<MZ041022> fix output error code
    vlc_output->err_code = 0;

    vlc_output->intra_mbs = 0;
    vlc_output->intraq_mbs = 0;
    vlc_output->acpred_mbs = 0;
    vlc_output->inter_mbs = 0;
    vlc_output->interq_mbs = 0;
    vlc_output->notcoded_mbs = 0;
    vlc_output->forward_mbs = 0;
    vlc_output->backward_mbs = 0;
    vlc_output->interpolate_mbs = 0;
    vlc_output->stuffing_mbs = 0;
    vlc_output->field_dct_mbs = 0;
    vlc_output->field_pred_mbs = 0;
    vlc_output->annex1 = 0;
    vlc_output->mv_bits = 0;
    vlc_output->tcoef_bits = 0;

    vlc_output->intra16x16_mbs = 0;
    vlc_output->intra4x4_mbs = 0;
    for (i=0; i<4; i++){
        vlc_output->intra16x16_type[i] = 0;
        vlc_output->intra_chroma_type[i] = 0;
    }
    for (i=0; i<9; i++){
        vlc_output->intra4x4_type[i] = 0;
    }

    return 0;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vop_header
 *
 *****************************************************************************/

int m4vac_vop_header(
                     m4vac_vlc_input *vlc_input)
{
    int len;
    int vop_coded, vop_type, vop_round, vop_quant;
    int vop_fcode_f, vop_fcode_b;
    int modulo_time_base, vop_time_inc, vop_time_inc_width;
    int interlaced, topf_first, alt_vscan;
    int ref_select_enc, ref_select_code;

    vop_coded = vlc_input->vop_coded;
    vop_type = vlc_input->vop_type;
    vop_round = vlc_input->vop_round;
    vop_quant = vlc_input->vop_quant;
    vop_fcode_f = vlc_input->fcode_f;
    vop_fcode_b = vlc_input->fcode_b;
    modulo_time_base = vlc_input->modulo_time_base;
    vop_time_inc = vlc_input->time_inc;
    vop_time_inc_width = vlc_input->time_inc_width;
    interlaced = vlc_input->interlaced;
    topf_first = vlc_input->topf_first;
    alt_vscan = vlc_input->alt_vscan;
    ref_select_enc = vlc_input->ref_select_enc;
    ref_select_code = vlc_input->ref_select_code;

    len = 0;

    len += putbits(0x000001B6, 32);		//vop_start_code
    len += putbits(vop_type, 2);		//vop_coding_type

    while (modulo_time_base >= 32){
        len += putbits(0xffffffff, 32);
        modulo_time_base -= 32;
    }
    if (modulo_time_base != 0){
        len += putbits(((1 << modulo_time_base) - 1), modulo_time_base);
    }
    len += putbits(0, 1);				//module_time_base termination
    len += putbits(1, 1);				//marker_bit
    len += putbits(vop_time_inc, vop_time_inc_width);	//vop_time_increment
    len += putbits(1, 1);				//marker_bit
    len += putbits(vop_coded, 1);		//vop_coded

    if (vop_coded){
        if (vop_type == 1){
            len += putbits(vop_round, 1);	//vop_round
        }
        len += putbits(0, 3);				//intra_dc_vlc_thr, encoder=0

        if (interlaced){
            len += putbits(topf_first, 1);	//top_field_first
            len += putbits(alt_vscan, 1);	//alternate_vertical_scan
        }

        len += putbits(vop_quant, 5);		//vop_quant
        if (vop_type != 0){
            len += putbits(vop_fcode_f, 3);	//vop_fcode_forward
        }
        if (vop_type == 2){
            len += putbits(vop_fcode_b, 3);	//vop_fcode_backward
        }
        //<AK040216>
        if (ref_select_enc){
            len += putbits(ref_select_code, 2);	//ref_select_code
        }

    }else{
#ifdef VLC_DEBUG2
        printf("VOP not coded. (next_start_code)\n");
#endif
        len += m4vac_next_start_code();
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_next_start_code
 *
 *****************************************************************************/

int m4vac_next_start_code()
{
    int offset, len;

    offset = getoffset();						//get offset_bit
    len = ((32 - (offset + 1)) & 0x7) + 1;

    return putbits((127 >> (8 - len)), len);	//put next_start_code
}

/******************************************************************************
 *
 * Function Name	: m4vac_interlaced_info
 *
 *****************************************************************************/

int m4vac_interlaced_info(
                          m4vac_vlc_input *vlc_input,
                          m4vac_vlc_output *vlc_output,
                          int mb_type,
                          int cbp)
{
    int len;
    int vop_type, field_dct, field_pred;
    int ftop_ref, fbot_ref, btop_ref, bbot_ref;

    vop_type = vlc_input->vop_type;
    field_dct = vlc_input->field_dct;
    field_pred = vlc_input->field_pred;
    ftop_ref = vlc_input->ftop_ref;
    fbot_ref = vlc_input->fbot_ref;
    btop_ref = vlc_input->btop_ref;
    bbot_ref = vlc_input->bbot_ref;

    len = 0;

    if ((mb_type == 3) || (mb_type == 4) || (cbp != 0)){
        len += putbits(field_dct, 1);			//dct_type

        if (field_dct){
            vlc_output->field_dct_mbs++;
        }
    }
    if (((vop_type == 1) && ((mb_type == 0) || (mb_type == 1)))
        || (vop_type == 2)){				//Encoder No Direct-mode

            len += putbits(field_pred, 1);			//field_prediction
            if (field_pred){
                if ((vop_type == 1) || ((vop_type == 2) && (mb_type != 9))){
                    len += putbits(ftop_ref, 1); //forward_top_field_reference
                    len += putbits(fbot_ref, 1); //forward_bottom_field_reference
                }
                if ((vop_type == 2) && (mb_type != 8)){
                    len += putbits(btop_ref, 1); //backward_top_field_reference
                    len += putbits(bbot_ref, 1); //backward_bottom_field_reference
                }
                vlc_output->field_pred_mbs++;
            }
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_mb_in_ivop
 *
 *****************************************************************************/

int m4vac_vlc_mb_in_ivop(
                         m4vac_vlc_input *vlc_input,
                         m4vac_vlc_output *vlc_output,
                         int dquant)
{
    int mb_type, cbp, i, len;
    int ac_pred_flag;
    int mb_posx, mb_posy, mb_sizex, mb_sizey;
    int stuff_bits, vop_minbit;
    int interlaced, short_vh;
    int tcoef_bits;
    int annex_i, dc_except;
    int annex_t, mb_quant;

    mb_posx = vlc_input->mb_posx;
    mb_posy = vlc_input->mb_posy;
    mb_sizex = vlc_input->mb_sizex;
    mb_sizey = vlc_input->mb_sizey;
    vop_minbit = vlc_input->vop_minbit;
    interlaced = vlc_input->interlaced;
    short_vh = vlc_input->short_vh;
    annex_i = vlc_input->annex_i;
    annex_t = vlc_input->annex_t;
    mb_quant = vlc_input->mb_quant;

    ac_pred_flag = vlc_input->ac_pred;
    len = 0;

    if ((mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)){
        stuff_bits = vop_minbit - nbits_vop;
        while (len < stuff_bits){
            len += m4vac_vlc_mcbpc(0, 5, 0);		//MCBPC:0000_0000_1
            vlc_output->stuffing_mbs++;				//generate StuffMB
        }
    }

    vlc_output->not_coded = 0;

    if (dquant == 0){
        mb_type = 3;		//INTRA
        vlc_output->intra_mbs++;
    }else{
        mb_type = 4;		//INTRA+Q
        vlc_output->intraq_mbs++;
        vlc_output->intra_mbs++;
    }
    vlc_mbinfo.mb_type = mb_type;

#ifdef VLC_DEBUG
    printf(" : ");
    switch (mb_type){
    case 0:	printf("INTER");	break;
    case 1:	printf("INTERQ");	break;
    case 3:	printf("INTRA");	break;
    case 4:	printf("INTRAQ");	break;
    }
    printf("\n");
#endif

    //<AK040121> Annex.I
    dc_except = !(short_vh & annex_i);
    cbp = 0;
    for (i=0; i<6; i++){
        if (vlc_input->coef_number[i] > dc_except){
            cbp |= (1 << (5 - i));
        }
    }
    vlc_mbinfo.cbp = cbp;

#ifdef VLC_DEBUG
    printf("QP = %d, CBP = %02x\n", vlc_input->mb_quant, cbp);
#endif

    len += m4vac_vlc_mcbpc(0, mb_type, cbp);			//MCBPC

    //<AK040122> Annex.I
    if (!short_vh){
        len += putbits(ac_pred_flag, 1);				//AC_PRED_FLAG
        if (ac_pred_flag){
            vlc_output->acpred_mbs++;
        }
    }else if (annex_i){									//Annex.I INTRA_MODE
        if (ac_pred_flag == 1){
            len += putbits(2, 2);
            vlc_output->acpred_mbs++;
        }else if (ac_pred_flag == 2){
            len += putbits(3, 2);
            vlc_output->acpred_mbs++;
        }else if (ac_pred_flag == 0){
            len += putbits(0, 1);
        }
    }

    len += m4vac_vlc_cbpy(1, cbp);						//CBPY

    if (mb_type == 4){
        //<AK040121> Annex.T
        if (short_vh && annex_t){
            len += m4vac_vlc_modified_dquant(mb_quant, dquant);		//DQUANT
        }else{
            len += m4vac_vlc_dquant(dquant);				//DQUANT
        }
    }
    if (!short_vh && interlaced){							//interlaced_information
        len += m4vac_interlaced_info(vlc_input, vlc_output, mb_type, cbp);
    }
    tcoef_bits = m4vac_vlc_intramb(vlc_input, vlc_output);	//TCOEFF
    len += tcoef_bits;
    vlc_output->tcoef_bits += tcoef_bits;

    vlc_output->q_not_coded = 0;

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_mb_in_pvop
 *
 *****************************************************************************/

int m4vac_vlc_mb_in_pvop(
                         m4vac_vlc_input *vlc_input,
                         m4vac_vlc_output *vlc_output,
                         int dquant)
{
    int mb_type, cbp, i, len;
    int not_coded, intra_mb, ac_pred_flag;
    int mb_posx, mb_posy, mb_sizex, mb_sizey;
    int stuff_bits, vop_minbit;
    int interlaced, field_pred, short_vh;
    int mv_bits, tcoef_bits;
    int annex_i, dc_except;
    int annex_t, mb_quant;

    mb_posx = vlc_input->mb_posx;
    mb_posy = vlc_input->mb_posy;
    mb_sizex = vlc_input->mb_sizex;
    mb_sizey = vlc_input->mb_sizey;
    vop_minbit = vlc_input->vop_minbit;
    interlaced = vlc_input->interlaced;
    field_pred = vlc_input->field_pred;
    short_vh = vlc_input->short_vh;
    annex_i = vlc_input->annex_i;
    annex_t = vlc_input->annex_t;
    mb_quant = vlc_input->mb_quant;

    intra_mb = (vlc_input->me_type == 0);
    ac_pred_flag = vlc_input->ac_pred;
    len = 0;

    if ((mb_posx == mb_sizex - 1) && (mb_posy == mb_sizey - 1)){
        stuff_bits = vop_minbit - nbits_vop;
        while (len < stuff_bits){
            len += putbits(0, 1);						//notcoded:0
            len += m4vac_vlc_mcbpc(1, 5, 0);			//mcbpc:0000_0000_1
            vlc_output->stuffing_mbs++;					//generate StuffMB
        }
    }

    //<AK040121> Annex.I
    dc_except = (intra_mb & !(short_vh & annex_i));
    cbp = 0;
    for (i=0; i<6; i++){
        if (vlc_input->coef_number[i] > dc_except){
            cbp |= (1 << (5 - i));
        }
    }
    vlc_mbinfo.cbp = cbp;
    not_coded = (!intra_mb && (cbp == 0) && !(interlaced && field_pred) &&
        (vlc_input->mv_fh0 == 0) && (vlc_input->mv_fv0 == 0));

    len += putbits(not_coded, 1);						//notcoded
    vlc_output->not_coded = not_coded;

    if (not_coded){
#ifdef NC_CHECK
        printf("Not Coded (%d, %d)\n", mb_posy, mb_posx);
#endif
        m4vac_vlc_mv_in_pvop(vlc_input, vlc_output, 1);	//Update Only
        vlc_output->notcoded_mbs++;
#ifdef VLC_DEBUG
        printf(" : NOTCODED\n");
#endif
        vlc_mbinfo.mb_type = 0;

    }else{
        if (intra_mb){
            if (dquant == 0){
                mb_type = 3;				//INTRA
                vlc_output->intra_mbs++;
            }else{
                mb_type = 4;				//INTRA+Q
                vlc_output->intraq_mbs++;
                vlc_output->intra_mbs++;
            }

        }else{
            if (dquant == 0){
                mb_type = 0;				//INTER
                vlc_output->inter_mbs++;
            }else{
                mb_type = 1;				//INTER+Q
                vlc_output->interq_mbs++;
                vlc_output->inter_mbs++;
            }
        }
        //<MZ040922> for mb_out
        vlc_mbinfo.mb_type = mb_type;

#ifdef VLC_DEBUG
        printf(" : ");
        switch (mb_type){
        case 0:	printf("INTER");	break;
        case 1:	printf("INTERQ");	break;
        case 3:	printf("INTRA");	break;
        case 4:	printf("INTRAQ");	break;
        }
        printf("\n");
        printf("QP = %d, CBP = %02x\n", vlc_input->mb_quant, cbp);
#endif

        len += m4vac_vlc_mcbpc(1, mb_type, cbp);				//MCBPC

        //<AK040122> Annex.I
        if (intra_mb){
            if (!short_vh){
                len += putbits(ac_pred_flag, 1);				//AC_PRED_FLAG
                if (ac_pred_flag){
                    vlc_output->acpred_mbs++;
                }
            }else if (annex_i){									//Annex.I INTRA_MODE
                if (ac_pred_flag == 1){
                    len += putbits(2, 2);
                    vlc_output->acpred_mbs++;
                }else if (ac_pred_flag == 2){
                    len += putbits(3, 2);
                    vlc_output->acpred_mbs++;
                }else if (ac_pred_flag == 0){
                    len += putbits(0, 1);
                }
            }
        }

        len += m4vac_vlc_cbpy(intra_mb, cbp);					//CBPY

        if ((mb_type == 1) || (mb_type == 4)){
            //<AK040121> Annex.T
            if (short_vh && annex_t){
                len += m4vac_vlc_modified_dquant(mb_quant, dquant);		//DQUANT
            }else{
                len += m4vac_vlc_dquant(dquant);				//DQUANT
            }
        }
        if (!short_vh && interlaced){				//interlaced_information
            len += m4vac_interlaced_info(vlc_input, vlc_output,  mb_type, cbp);
        }

        if (intra_mb){
            m4vac_vlc_mv_in_pvop(vlc_input, vlc_output, 1);	//Update Only
            tcoef_bits = m4vac_vlc_intramb(vlc_input, vlc_output);	//TCOEFF
            mv_bits = 0;
        }else{
            mv_bits = m4vac_vlc_mv_in_pvop(vlc_input, vlc_output, 0);	//MV
            tcoef_bits = m4vac_vlc_intermb(vlc_input, vlc_output);	//TCOEFF
        }
        len += mv_bits + tcoef_bits;
        vlc_output->mv_bits += mv_bits;
        vlc_output->tcoef_bits += tcoef_bits;
    }

    if (vlc_input->vp_enable && (vlc_input->vp_mb_num == 0)
        && (!(vlc_input->mb_posx == 0 && vlc_input->mb_posy == 0))) {
            vlc_output->q_not_coded = 0;
    } else {
        if (not_coded){
            vlc_output->q_not_coded = 1;
        }else{
            vlc_output->q_not_coded = 0;
        }
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_mb_in_bvop
 *
 *****************************************************************************/

int m4vac_vlc_mb_in_bvop(
                         m4vac_vlc_input *vlc_input,
                         m4vac_vlc_output *vlc_output,
                         int dquant)
{
    int i, len;
    int coloc_not_coded, mb_type;
    int cbp = 0, interlaced;
    int mv_bits, tcoef_bits;

    len = 0;
    coloc_not_coded = vlc_input->coloc_not_coded;
    mb_type = vlc_input->me_type + 7;	//8:forward,9:backward,10:interpolate
    interlaced = vlc_input->interlaced;

    if (coloc_not_coded){
        m4vac_vlc_mv_in_bvop(vlc_input, vlc_output, 1);	//Update Only
        vlc_output->notcoded_mbs++;
#ifdef VLC_DEBUG
        printf(" : COLOC_NC\n");
#endif
        vlc_mbinfo.mb_type = 5;
        vlc_mbinfo.cbp = 0;

    }else{
        cbp = 0;
        for (i=0; i<6; i++){
            if (vlc_input->coef_number[i] > 0){
                cbp |= (1 << (5 - i));
            }
        }
        vlc_mbinfo.cbp = cbp;
        if (cbp == 0){					//modb
            len += putbits(1, 2);
        }else{
            len += putbits(0, 2);
        }

        switch (mb_type){				//mb_type
        case 8:		//Forward
            len += putbits(1, 4);
            vlc_output->forward_mbs++;
            vlc_output->inter_mbs++;
            break;
        case 9:		//Backward
            len += putbits(1, 3);
            vlc_output->backward_mbs++;
            vlc_output->inter_mbs++;
            break;
        case 10:	//Interpolate
            len += putbits(1, 2);
            vlc_output->interpolate_mbs++;
            vlc_output->inter_mbs++;
            break;
        default:
            printf("ERROR: ME_TYPE!!\n");
        }
        vlc_mbinfo.mb_type = mb_type - 3;

#ifdef VLC_DEBUG
        printf(" : ");
        switch (mb_type){
        case 8:	printf("FORWARD");	break;
        case 9:	printf("BACKWARD");	break;
        case 10:	printf("INTERPOLATE");	break;
        }
        printf("\n");
        if (cbp != 0)
            printf("QP = %d, ", vlc_input->mb_quant);
        printf("CBP = %02x", cbp);
        printf("\n");
#endif

        if (cbp != 0){
            len += putbits(cbp, 6);		//cbpb
            len += m4vac_vlc_dbquant(dquant);					//DBQUANT
        }
        if (interlaced){						//interlaced_information
            len += m4vac_interlaced_info(vlc_input, vlc_output, mb_type, cbp);
        }

        mv_bits = m4vac_vlc_mv_in_bvop(vlc_input, vlc_output, 0);	//MV
        tcoef_bits = m4vac_vlc_intermb(vlc_input, vlc_output);		//TCOEFF
        //printf("MB(%d, %d) : MV=%d bits, TCOEF=%d bits\n", vlc_input->mb_posy, vlc_input->mb_posx, mv_bits, tcoef_bits);
        //printf("MB(%d, %d) : TCOEF=%d bits\n", vlc_input->mb_posy, vlc_input->mb_posx, tcoef_bits);
        len += mv_bits + tcoef_bits;
        vlc_output->mv_bits += mv_bits;
        vlc_output->tcoef_bits += tcoef_bits;
    }

    if (vlc_input->vp_enable && (vlc_input->vp_mb_num == 0)
        && (!(vlc_input->mb_posx == 0 && vlc_input->mb_posy == 0))) {
            vlc_output->q_not_coded = 0;
    } else {
        if (coloc_not_coded || (cbp == 0)){
            vlc_output->q_not_coded = 1;
        }else{
            vlc_output->q_not_coded = 0;
        }
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_mcbpc
 *
 *****************************************************************************/

int m4vac_vlc_mcbpc(
                    int vop_type,
                    int mb_type,
                    int cbp)
{
    int len = 0;

    cbp &= 0x3;		//CBPC

    if (vop_type == 0){			//I-VOP
        switch (mb_type){
        case 3:						//Intra
            switch (cbp){
            case 0:
                len = putbits(1, 1);
                break;
            case 1:
                len = putbits(1, 3);
                break;
            case 2:
                len = putbits(2, 3);
                break;
            case 3:
                len = putbits(3, 3);
                break;
            default:
                //do nothing
                break;
            }
            break;
        case 4:						//Intra+Q
            switch (cbp){
            case 0:
                len = putbits(1, 4);
                break;
            case 1:
                len = putbits(1, 6);
                break;
            case 2:
                len = putbits(2, 6);
                break;
            case 3:
                len = putbits(3, 6);
                break;
            default:
                //do nothing
                break;
                }
            break;
        default:					//Stuffing
            len = putbits(1, 9);
            break;
        }
    }else{						//P-VOP
        switch (mb_type){
        case 0:
            switch (cbp){
            case 0:
                len = putbits(1,1);
                break;
            case 1:
                len = putbits(3,4);
                break;
            case 2:
                len = putbits(2,4);
                break;
            case 3:
                len = putbits(5,6);
                break;
            default:
                //do nothing
                break;
            }
            break;
        case 1:
            switch (cbp){
            case 0:
                len = putbits(3,3);
                break;
            case 1:
                len = putbits(7,7);
                break;
            case 2:
                len = putbits(6,7);
                break;
            case 3:
                len = putbits(5,9);
                break;
            default:
                //do nothing
                break;
            }
            break;
        case 2:
            switch (cbp){
            case 0:
                len = putbits(2,3);
                break;
            case 1:
                len = putbits(5,7);
                break;
            case 2:
                len = putbits(4,7);
                break;
            case 3:
                len = putbits(5,8);
                break;
            default:
                //do nothing
                break;
            }
            break;
        case 3:
            switch (cbp){
            case 0:
                len = putbits(3,5);
                break;
            case 1:
                len = putbits(4,8);
                break;
            case 2:
                len = putbits(3,8);
                break;
            case 3:
                len = putbits(3,7);
                break;
            default:
                //do nothing
                break;
            }
            break;
        case 4:
            switch (cbp){
            case 0:
                len = putbits(4,6);
                break;
            case 1:
                len = putbits(4,9);
                break;
            case 2:
                len = putbits(3,9);
                break;
            case 3:
                len = putbits(2,9);
                break;
            default:
                //do nothing
                break;
            }
            break;
        default:
            len = putbits(1,9);
            break;
        }
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_cbpy
 *
 *****************************************************************************/

int m4vac_vlc_cbpy(
                   int intra_mb,
                   int cbp)
{
    int len = 0;

    cbp >>= 2;		//CBPY
    if (!intra_mb){
        cbp = (~cbp) & 0xf;
    }

    switch (cbp){
    case 0:
        len = putbits(3, 4);
        break;
    case 1:
        len = putbits(5, 5);
        break;
    case 2:
        len = putbits(4, 5);
        break;
    case 3:
        len = putbits(9, 4);
        break;
    case 4:
        len = putbits(3, 5);
        break;
    case 5:
        len = putbits(7, 4);
        break;
    case 6:
        len = putbits(2, 6);
        break;
    case 7:
        len = putbits(11, 4);
        break;
    case 8:
        len = putbits(2, 5);
        break;
    case 9:
        len = putbits(3, 6);
        break;
    case 10:
        len = putbits(5, 4);
        break;
    case 11:
        len = putbits(10, 4);
        break;
    case 12:
        len = putbits(4, 4);
        break;
    case 13:
        len = putbits(8, 4);
        break;
    case 14:
        len = putbits(6, 4);
        break;
    case 15:
        len = putbits(3, 2);
        break;
    default:
        break;
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_dquant
 *
 *****************************************************************************/
int m4vac_vlc_dquant( int dquant )
{
    // dquant is allowed [-2,-1,1,2]
    int len;

    switch(dquant) {
    case -1:
        len = putbits(0, 2);
        break;
    case -2:
        len = putbits(1, 2);
        break;
    case 1:
        len = putbits(2, 2);
        break;
    case 2:
        len = putbits(3, 2);
        break;
    default:
        //#ifdef DQUANT_DEBUG
        printf("invalid dquant value!! exit.\n");
        exit(1);
        //#endif
        break;
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_dbquant
 *
 *****************************************************************************/
int m4vac_vlc_dbquant( int dbquant )
{
    // dbquant is allowed [-2,0,2]
    int len;

    switch(dbquant) {
    case -2:
        len = putbits(2, 2);
        break;
    case 0:
        len = putbits(0, 1);
        break;
    case 2:
        len = putbits(3, 2);
        break;
    default:
        //#ifdef DQUANT_DEBUG
        printf("invalid dbquant value!! exit.\n");
        exit(1);
        //#endif
        break;
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_raw_output
 *
 *****************************************************************************/
//<AK040123> JPEG

int m4vac_vlc_raw_output(
                         m4vac_vlc_input *vlc_input)
{
    int i, len, iter;
    unsigned long *coef_in, coef_h, coef_l;
    unsigned long data;

    len = 0;
    coef_in = (unsigned long *)vlc_input->coef_in;

    if (vlc_input->jpeg_yc == 1){
        iter = 128;
    }else{
        iter = 64;
    }

    for (i=0; i<iter; i++){
        coef_h = *coef_in++;
        coef_l = *coef_in++;
        data = ((coef_h << 16) | coef_l);

        len += putbits(data, 32);
    }

    return len;
}

/******************************************************************************
 *
 * Function Name	: m4vac_change_partition
 *
 *****************************************************************************/

int m4vac_change_partition(int part)
{
    switch (part){
    case 1:
        putpart = 0;
        break;
    case 2:
        putpart = 1;
        break;
    case 3:
        putpart = 2;
        break;
    default:
        printf("No such partition: %d\n", part);
    }
    return putpart;
}

/******************************************************************************
 *
 * Function Name	: bufreset
 *
 *****************************************************************************/

//<IM031225> for slice header
//int bufreset(long *stream)
//<AK040123> for STREAM_RW
//int bufreset(long *stream, int bit_offset)
int bufreset(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output)
{
    int i, bit_offset;
    long byte_offset;
    //	long* tmp;
    //<IM031225>
    unsigned long bit_tmp = 0;
    unsigned long bit_mask;

#ifdef VLC_DEBUG2
    printf("Buffer Reset\n");
#endif
    strrw_input.mode		= STRRW_ENCODE;
    strrw_input.endian		= vlc_input->strm_buf_endian;
    strrw_input.strs_addr	= (long)vlc_output->stream_out;
    strrw_input.stre_addr	= vlc_input->stream_end;

    //	byte_offset = (long)stream % 4;
    byte_offset = ((long)vlc_output->stream_out) % 4;
    bit_offset = vlc_input->bit_offset;

    offset_long = 0;
    offset_long2 = 0;
    offset_long3 = 0;
    for (i=0; i<3; i++){
        offset_bitp[i] = 0;
        bufp[i] = 0;
    }

#ifdef iVCP1_VLC_HM
    //ems_buf_bit = (byte_offset << 3) | vlc_input->bit_offset;
#endif
    //<IM031225>
    offset_bitp[0] = byte_offset*8 + bit_offset;
    //<AK040123>
    m4vac_stream_rw(&strrw_input, &strrw_output, STRRW_INIT);
    bufp[0] = strrw_output.read_data;
    //	tmp = (long *)((long)stream & 0xFFFFFFFC);
    //	bufp[0] =  (unsigned long)*tmp;
    //<IM031225>
    switch(bit_offset){
    case(0):bit_tmp = 0x00; break;
    case(1):bit_tmp = 0x80; break;
    case(2):bit_tmp = 0xC0; break;
    case(3):bit_tmp = 0xE0; break;
    case(4):bit_tmp = 0xF0; break;
    case(5):bit_tmp = 0xF8; break;
    case(6):bit_tmp = 0xFC; break;
    case(7):bit_tmp = 0xFE; break;
    default:printf("(VLC) bit_offset error\n");break;
    }

    //	bufp[0] =  (unsigned long)*tmp;
    switch ( byte_offset ) {
        case 0:
            bit_mask = (bit_tmp << 24);
            //			bit_mask = bit_tmp;
            bufp[0] &= bit_mask;
            //			bufp[0] &= 0x00000000;
            break;
        case 1:
            bit_mask = 0xFF000000	| (bit_tmp << 16);
            //			bit_mask = (bit_tmp<<8)  | 0x000000FF;
            bufp[0] &= bit_mask;
            //			bufp[0] &= 0x000000FF;
            break;
        case 2:
            bit_mask = 0xFFFF0000	| (bit_tmp << 8);
            //			bit_mask = (bit_tmp<<16) | 0x0000FFFF;
            bufp[0] &= bit_mask;
            //			bufp[0] &= 0x0000FFFF;
            break;
        case 3:
            bit_mask = 0xFFFFFF00	| bit_tmp;
            //			bit_mask = (bit_tmp<<24) | 0x00FFFFFF;
            bufp[0] &= bit_mask;
            //			bufp[0] &= 0x00FFFFFF;
            break;
        default:
            //do nothing
            break;
    }
    //<IZU030311>
    //		endian_set(&bufp[0],strm_buf_endian);
    //#ifdef PC
    //		endian_set(&bufp[0],3);
    //#endif

    return 0;
}

/******************************************************************************
 *
 * Function Name	: bufflush
 *
 *****************************************************************************/

int bufflush()
{
#ifdef VLC_DEBUG2
    printf("Buffer Flush\n");
#endif
    if (offset_bitp[0] != 0){
#ifdef VLC_DEBUG3
        printf("->output %08lX\n", bufp[0]);
#endif
        //<IZU030311>
        //		endian_set(&bufp[0],strm_buf_endian);
        //#ifdef PC
        //		endian_set(&bufp[0],3);
        //#endif
        //		stream_buf[offset_long++] = bufp[0];
        //<AK040123> for STREAM_RW
        strrw_input.write_data	= bufp[0];
        m4vac_stream_rw(&strrw_input, &strrw_output, STRRW_NORMAL);
        offset_long++;
    }
    m4vac_stream_rw(&strrw_input, &strrw_output, STRRW_FLUSH);
    offset_bitp[0] = 0;
    bufp[0] = 0;
#ifdef iVCP1_VLC_HM
    ems_buf_bit = 0;
#endif

    return 0;
}

/******************************************************************************
 *
 * Function Name	: getoffset
 *
 *****************************************************************************/

int getoffset()
{
    return offset_bitp[0];
}

/******************************************************************************
 *
 * Function Name	: concatdp
 *
 *****************************************************************************/

int concatdp()
{
    int i, end, resid, savepart;

    savepart = putpart + 1;
    m4vac_change_partition(1);

    end = offset_long2;
    for (i=0; i<end; i++){
        offset_long2--;
        putbits(dp_ram[i], 32);
    }

    resid = offset_bitp[1];
    if (resid != 0){
        offset_bitp[1] = 0;
        putbits((bufp[1] >> (32 - resid)), resid);
    }

    end = offset_long3;
    for (i=0; i<end; i++){
        offset_long3--;
        putbits(dp_ram[DPRAM_SIZE-1 - i], 32);
    }

    resid = offset_bitp[2];
    if (resid != 0){
        offset_bitp[2] = 0;
        putbits((bufp[2] >> (32 - resid)), resid);
    }

    m4vac_change_partition(savepart);
    offset_long2 = offset_long3 = 0;
    offset_bitp[1] = offset_bitp[2] = 0;
    bufp[1] = bufp[2] = 0;

    return 0;
}

/******************************************************************************
 *
 * Function Name	: dpdisp
 *
 *****************************************************************************/

int dpdisp()
{
#ifdef VP_DEBUG
    printf("--- STR_RAM ---\n");
    char_out((unsigned char *)stream_buf, offset_long*4);
    printf(" + %08lX (%d bits)\n", bufp[0], offset_bitp[0]);
    printf("--- DP_RAM ---\n");
    char_out((unsigned char *)dp_ram, DPRAM_SIZE*4);
    printf(" + %08lX (%d bits)\n", bufp[2], offset_bitp[1]);
    printf(" + %08lX (%d bits)\n", bufp[2], offset_bitp[2]);
#endif
    return 0;
}

/******************************************************************************
 *
 * Function Name	: dpbits
 *
 *****************************************************************************/

int dpbits()
{
#ifdef VP_DEBUG
    printf("DP1 = %d, DP2 = %d, DP3 = %d, TOTAL:%d, VOP:%d\n",
        offset_long*32+offset_bitp[0], offset_long2*32+offset_bitp[1],
        offset_long3*32+offset_bitp[2],
        offset_long*32+offset_bitp[0] + offset_long2*32+offset_bitp[1]
    + offset_long3*32+offset_bitp[2], nbits_vop);
#endif
    return 0;
}

/******************************************************************************
 *
 * Function Name	: m4vac_dp2_close
 *
 *****************************************************************************/

int m4vac_dp2_close(int vop_type)
{
    int dp2_close;

    if (vop_type == 0){
        if (offset_long2 + 2 > DPRAM_SIZE - 1 - offset_long3){
            dp2_close = 1;
        }else{
            dp2_close = 0;
        }
    }else if (vop_type == 1){
        if (offset_long2 + 5 > DPRAM_SIZE - 1 - offset_long3){
            dp2_close = 1;
        }else{
            dp2_close = 0;
        }
    }else{
        dp2_close = 0;
    }

    return dp2_close;
}

/******************************************************************************
 *
 * Function Name	: putbits
 *
 *****************************************************************************/

int putbits(unsigned int value, int length)
{
    int offset_bit, shifted_bit;
    unsigned long buf1, buf2;
    static int now_dp_transfer = 0;
#ifdef VLC_DEBUG3
    static int nbits = 0;
#endif
    int i, j, check_from, check_to;
    static int pre_zeros = 0;
    unsigned char byte_seq[10];

    if (length <= 0 || length > 32){
        printf("ERROR: LENGTH!! putbits(%d, %d), exit.\n", value, length);
        exit (1);
    }

#ifdef BITLOG
    for (i=0; i<length; i++){
        printf("%d", ((value & (1 << (length - i - 1))) != 0));
    }
    printf(" :%d bit -> (%d)\n", length, putpart+1);
    //	printf("%08X :%d bit -> (%d)\n", value, length, putpart+1);
#endif

#ifdef VLC_DEBUG3
    for (i=0; i<length; i++){
        printf("%d", ((value & (1 << (length - i - 1))) != 0));
    }
    printf(" :%d bit -> (%d)\n", length, putpart+1);

    //	printf("%08X :%d bit -> (%d)\n", value, length, putpart+1);
    //	nbits += length;
    //	printf("nbits => %d\n", nbits);
#endif

    offset_bit = offset_bitp[putpart];
    buf1 = bufp[putpart];

    //<AK040329> hard insert EPB
    value <<= (32 - length);
    shifted_bit = offset_bit + length;
    buf1 |= (value >> offset_bit);
    if (offset_bit == 0){
        buf2 = 0;
    }else{
        buf2  = (value << (32 - offset_bit));
    }
#ifdef iVCP1_VLC_HM
    //Only count emulation prevention byte to current MB bits if valid bits (shifted_bit) in buffer larger then or equal to 32bits
    ems_buf_bit += length;
    if(ems_buf_bit >= EMS_SWAP_BIT) {
        inserted_bit += inserted_bit_buf;
        inserted_bit_buf = 0;
    }
#endif

    if (hard_insert){
        //pre_process
        check_from = offset_bit / 8;
        check_to   = shifted_bit / 8;

        if (check_from < check_to){
            if (pre_zeros == 2){
                byte_seq[0] = byte_seq[1] = 0;
            }else if (pre_zeros == 1){
                byte_seq[0] = 0xff;
                byte_seq[1] = 0;
            }else{
                byte_seq[0] = byte_seq[1] = 0xff;
            }
            for (i=0; i<4; i++){
                byte_seq[i+2] = (char)((buf1 >> ((3 - i)*8)) & 0xff);
                byte_seq[i+6] = (char)((buf2 >> ((3 - i)*8)) & 0xff);
            }

            for (i=check_from+2; i<check_to+2; i++){
                //<MZ041102> add insert at 0x00, 0x02
                //				if ((byte_seq[i-2] == 0) && (byte_seq[i-1] == 0)
                //					 && ((byte_seq[i] == 0x01) || (byte_seq[i] == 0x03))){
                if ((byte_seq[i-2] == 0) && (byte_seq[i-1] == 0)
                    && ((byte_seq[i] == 0x00) || (byte_seq[i] == 0x01) ||
                    (byte_seq[i] == 0x02) || (byte_seq[i] == 0x03))){
#ifndef LITE_LOG
                        printf("Hardware Emulation_Prevention_Byte Insertion.\n");
#endif		
                        for (j=8; j>=i; j--){
                            byte_seq[j+1] = byte_seq[j];
                        }
                        byte_seq[i] = 0x03;
#ifdef iVCP1_VLC_HM
                        //Only count emulation prevention byte to current MB bits if valid bits (shifted_bit) in buffer larger then or equal to 32bits
                        if(ems_buf_bit >= EMS_SWAP_BIT) {
                            inserted_bit += 8;
                        }else{
                            inserted_bit_buf += 8;
                        }
                        shifted_bit += 8;
#else
                        shifted_bit += 8;
                        inserted_bit += 8;
#endif

                        for (j=0; j<4; j++){
                            buf1 = (buf1 << 8) | byte_seq[j+2];
                            buf2 = (buf2 << 8) | byte_seq[j+6];
                        }
                        break;
                }
            }
        }
    }

    if (shifted_bit >= 32){
#ifdef VLC_DEBUG3
        printf("->output part%d %08lX\n", putpart+1, buf1);
#endif
#ifdef BITLOG
        printf("->output %08lX\n", buf1);
#endif

        if (hard_insert){
            //post_process
            if ((buf1 & 0xffff) == 0){
                pre_zeros = 2;
            }else if ((buf1 & 0xff) == 0){
                pre_zeros = 1;
            }else{
                pre_zeros = 0;
            }
        }

        if (putpart == 0){
            strrw_input.write_data	= buf1;
            m4vac_stream_rw(&strrw_input, &strrw_output, STRRW_NORMAL);
            offset_long++;

        }else if (putpart == 1){
            dp_ram[offset_long2++] = (long)buf1;
        }else if (putpart == 2){
            dp_ram[DPRAM_SIZE-1 - offset_long3] = (long)buf1;
            offset_long3++;
        }

        offset_bit = shifted_bit - 32;
        buf1 = buf2;
    }else{
        offset_bit = shifted_bit;
    }

#ifdef iVCP1_VLC_HM
    if(ems_buf_bit >= EMS_SWAP_BIT) {
        ems_buf_bit -= EMS_SWAP_BIT;
    }
#endif

    offset_bitp[putpart] = offset_bit;
    bufp[putpart] = buf1;

    if ((offset_long3 + offset_long2 == DPRAM_SIZE - 1) && (putpart == 2)
        && (now_dp_transfer == 0)){

            dp3_close = 1;
            dpbits();
            now_dp_transfer = 1;
            concatdp();
            now_dp_transfer = 0;
            dpbits();
            m4vac_change_partition(1);
    }

    return length;
}

/******************************************************************************
 *
 * Function Name	: m4vac_vlc_scan
 *
 *****************************************************************************/
//<AK040122> Annex.I / JPEG
void m4vac_vlc_scan(m4vac_vlc_input *vlc_input)
{
    int i, scan_mode, ac_pred, pred_mb_direc;
    int pred_direc[6];
    long *coef_in, coef_buf[384];
    int h264;
    int short_vh, annex_i;
    int jpeg_mode, jpeg_yc, jpeg_direc;

    coef_in  = (long *)vlc_input->coef_in;
    scan_mode = vlc_input->pred_scan_mode;
    ac_pred = vlc_input->ac_pred;
    pred_mb_direc = vlc_input->pred_mb_direc;
    h264 = vlc_input->h264;
    short_vh = vlc_input->short_vh;
    annex_i = vlc_input->annex_i;
    jpeg_mode = vlc_input->jpeg_mode;
    jpeg_yc = vlc_input->jpeg_yc;

    if (vlc_input->noscan == 0){
        if (h264){
            //scan is in residual()
        }else if (jpeg_mode){
            if (jpeg_yc == 1){
                for (i=0; i<256; i++){
                    coef_buf[i] = coef_in[i];
                }
            }else{
                for (i=0; i<128; i++){
                    coef_buf[i] = coef_in[i];
                }
            }
        }else{
            for (i=0; i<384; i++){
                coef_buf[i] = coef_in[i];
            }
        }

        if (h264){
            //scan is in residual()
        }else if (jpeg_mode){
            if (scan_mode == 1){		//force vert
                jpeg_direc = 2;
            }else if (scan_mode == 2){	//force_hori
                jpeg_direc = 1;
            }else{
                jpeg_direc = 0;			//zgzg
            }

            if (jpeg_yc){
                for (i=0; i<4; i++){
                    m4vac_scan_blk(&coef_buf[i*64], &coef_in[i*64], jpeg_direc);
                }
            }else{
                for (i=0; i<2; i++){
                    m4vac_scan_blk(&coef_buf[i*64], &coef_in[i*64], jpeg_direc);
                }
            }
        }else{
            for (i=0; i<6; i++){
                switch (scan_mode){
                case 0:
                    if (short_vh && annex_i){
                        pred_direc[i] = (ac_pred != 0) ? (ac_pred == 1) ?
                            1 : 2 : 0 ;	// 0: izgzg 1: ihori 2: ivert
                    }else{
                        pred_direc[i] = (ac_pred != 0) ? ((pred_mb_direc >> i) & 0x1) ?
                            1 : 2 : 0 ;	// 0: izgzg 1: ihori 2: ivert
                    }
                    break;
                case 1:
                    pred_direc[i] = 2;
                    break;
                case 2:
                    pred_direc[i] = 1;
                    break;
                default:
                    pred_direc[i] = 0;
                    break;
                }
                m4vac_scan_blk(&coef_buf[i*64], &coef_in[i*64], pred_direc[i]);
            }
        }
    }
}

/******************************************************************************
 *
 * Function Name	: m4vac_scan_blk
 *
 *****************************************************************************/

int m4vac_scan_blk(
                   long *coef_buf,
                   long *coef_out,
                   int direc)
{
    int i, zeros;
    long coef;
    const int *scan;


    switch (direc){
    case 0:		//No AC_PRED
        scan = izgzg;
        break;
    case 1:		//Refer Above
        scan = ihori;
        break;
    case 2:		//Refer Left
        scan = ivert;
        break;
    case 3:		//No Scan
        scan = inoscan;
        break;
    default:
        scan = NULL;
        printf("ERROR SCAN DIRECTION: %d\n", direc);
        break;
    }

    zeros = 0;
    for (i=0; i<64; i++){
        //printf(" %03x", coef_buf[i]&0x0fff);
        coef = coef_buf[scan[i]];
        coef_out[i] = coef;
        // FOR DEBUG
        //coef_out[i] = coef_buf[i];
        //printf("SCAN: %d %03x\n", scan[i],coef_out[i]&0x0fff);
        if (coef)
            zeros = 0;
        else
            zeros++;
    }

    return 64 - zeros;
}


/******************************************************************************
 *
 * Function Name	: m4vac_vlc_swap_dp
 *
 *****************************************************************************/

unsigned long m4vac_vlc_swap_dp(
                                unsigned long lword,
                                int endian)
{
    switch(endian){
        case 1:
            lword = ((lword & 0xffff0000) >> 16) | ((lword & 0x0000ffff) << 16);
            break;
        case 2:
            lword = ((lword & 0xff000000) >> 8) | ((lword & 0x00ff0000) << 8) |
                ((lword & 0x0000ff00) >> 8) | ((lword & 0x000000ff) << 8);
            break;
        case 3:
            lword = ((lword & 0xff000000) >> 24) | ((lword & 0x00ff0000) >> 8) |
                ((lword & 0x0000ff00) << 8) | ((lword & 0x000000ff) << 24);
            break;
        default:
            break;
    }
    //MZ130306
#ifdef EWS_LINUX
    lword = ((lword & 0xff000000) >> 24) | ((lword & 0x00ff0000) >> 8) |
        ((lword & 0x0000ff00) << 8) | ((lword & 0x000000ff) << 24);
#endif

    return lword;
}

    // == end vlc.c
    
    
 /****************************************************************************/
/*																			*/
/*	MPEG4core Cmodel														*/
/*																			*/
/*	Copyright (C) Renesas Technology Corp., 2003. All rights reserved.		*/
/*																			*/
/*	Version  1.0 : m4vac_vlc.h					2003/03/11 12:00			*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*	Modification history													*/
/*	Ver.1.00	2003/03/11	start codes										*/
/*																			*/
/****************************************************************************/

int vlc_logout;
#define VLCLOG	if(vlc_logout)printf

//#define STREAM_MAX	1024*64/8/4*20
//<MZ040526> not need yet
//#define STREAM_MAX	1024*64/8/4*20*2
//Define VLC Process Type
#define	BUF_RESET	0
#define	BUF_FLUSH	1
#define	VOP_HEADER	2
#define	MB_ENCODE	3
#define	VOP_FOOTER	4
#define	RAW_OUTPUT	5
#define	VOP_FILLER	6

//for motion_vector prediction
#define MV_INVALID ((short)0x8000)
#define RF_INVALID -2
//for total_coeff prediction
#define TC_UNAVAIL -2
//for intra_4x4 prediction
#define IT_DCONLY	-2
//#define IT_UNAVAIL	-1
#define IT_UNAVAIL	2

//error code
#define ERRE_AVC_MB_TYPE	0xfa010000
#define ERRE_AVC_MB_QUANT	0xfa020000
#define ERRE_AVC_REFIDX		0xfa030000
#define ERRE_AVC_PCM_ZERO	0xfa040000
#define ERRE_VOP_SIZE		0xff000000
#define ERRE_COEF_CLIP		0xff100000

#ifdef iVCP1_VLC_HM
#ifdef iVCP1E_HM_SPEC
#define EMS_SWAP_BIT    48      //for iVCP1M/iVCP1E
#else
#define EMS_SWAP_BIT    32      //for iVCP1
#endif
#endif

#define Y_DC_IDX    32
#define U_DC_IDX    33
#define V_DC_IDX    34
typedef struct {

    //VOL level
    //	long	low_delay;			//VLD only
    long	time_inc_width;
    long	mb_sizex;
    long	mb_sizey;
    long	mb_num_bit;
    long	interlaced;
    long	short_vh;
    long	vp_enable;
    long	vp_limit;
    long	vp_mb_num;
    long	hext_enable;
    long	dp_enable;
    long	rvlc_enable;

    //VOP level
    long	vop_type;
    long	modulo_time_base;
    long	time_inc;
    long	vop_coded;
    long	vop_round;
    long	topf_first;
    long	alt_vscan;
    long	vop_quant;
    long	short_vh_indicator;
    long	source_format;
    long	gob_frame_id;
    long	fcode_f;
    long	fcode_b;
    //	long	intra_dc_vlc_thr;	//VLD only

    //MB level
    long	me_type;
    long	coef_number[8]; //4:2:2 8blk 8x8
    long	ac_pred;
    long	mb_quant;
    long	mb_posx;
    long	mb_posy;
    long	mv_fh0;
    long	mv_fv0;
    long	mv_fh1;
    long	mv_fv1;
    //<MZ031211> add P_8x8
    long	mv_fh2;
    long	mv_fv2;
    long	mv_fh3;
    long	mv_fv3;
    long	mv_bh0;
    long	mv_bv0;
    long	mv_bh1;
    long	mv_bv1;
    //<MZ031211> add P_8x8
    long	mv_bh2;
    long	mv_bv2;
    long	mv_bh3;
    long	mv_bv3;
    long	coloc_not_coded;
    long	coloc_not_coded_next;
    long	field_dct;
    long	field_pred;
    long	ftop_ref;
    long	fbot_ref;
    long	btop_ref;
    long	bbot_ref;
    long	pred_mb_direc;	// 03/02/17 added
    long	pred_scan_mode;	// 03/02/17 added

    //CTRL
    long*	coef_in;
    long	process_type;
    long	vop_minbit;		//<AK050117>
    long	noscan;		// 03/02/17 add
    long	strm_buf_endian;
    //<MZ041027> adopt endian of mbout
    long	dp_ram_endian;
    //miche
    long	h264;
#ifdef iVCP1_VLC_HM
    long    profile_idc;
    long    fms_flag;   //first_mb_in_slice flag
    long    mb_target;
    long    filler_size;
    long    transform_size_8x8_flag;
#endif
    //<MZ040108> adopt slice encoding
    long	vp_mbs_limit;
    //<MZ040108> add I_PCM
    long	intra_pcm;
    //<MZ040212> adopt constrained_intra_pred
    long	constrained_intra_pred;
    //<MZ040106> add P_16x8, P_8x16
    //	long	intra_16or4;
    //<MZ031210> add P_8x8
    //	long	inter_16x16;
    long	mb_part;                //0: 16x16, 1: 4x4, 2: 8x8
    long	intra_luma_type[16];
    long	intra_chroma_type;
    //<MZ040107> adopt multi reference
    long	refidx[4];
    //<MZ040218> add ref_idx_last
    long	ref_idx_last;
    //<MZ041021> adopt coef_clip info out
    long	coef_clip;
    //<MZ041029> modify pcm_zero info out
    long	pcm_zero;
    long    chroma_format_idc; //HungCao140806
    long    ctrl_bit_depth_y;   //MZ121022

    //<MZ040608> add MB-info output from FME
    long	sad;
    long	mad;
    long	mxd;
    long	force_skip;
    //<MZ040910>
    long	mb_out;
    long	*dp2_addr;

    //<IM040223> remove
    //	long	deblock_disable;
    //	long	slice_alpha;
    //	long	slice_beta;
    //<IM031225>
    long	bit_offset;
    long	emu_prev_byte;

    //<AK040121> H.263 profile3
    long	plusptype;
    long	opptype;
    long	annex_i;
    long	annex_j;
    long	annex_k;
    long	annex_t;
    long	par;
    long	epar;
    long	custom_pcf;
    long	cpcfc;
    //<AK040615> Profile3 adopt for REG (VP4_VLC_PIC, VP4_VLC_CLK, VP4_263_CTRL)
    long	custom_pic;			//source_format==6
    long	width_indicator;	//(mb_sizex*4)-1
    long	height_indicator;	//(mb_sizey*4)
    long	gfid_first;			//adopt for NEC
    long	mpeg4_pmv;			//use MPEG4-mode pmv in H.263
    //<AK040123> JPEG
    long	jpeg_mode;
    long	jpeg_yc;
    //<AK040216> Temporal_Scalability
    long	ref_select_enc;
    long	ref_select_code;
    //<AK040123> for STREAM_RW
    long	stream_end;
} m4vac_vlc_input;

typedef struct {
    long*	stream_out;
    //	long*	nc_info;	//output not_coded in hardware
    long	not_coded;
    long	q_not_coded;
    long	nbits_mb;
    long	nbits_vop;
    //<MZ040721> add number of inserted bits by emulation_prevention 
    long	nbits_ins;
    long	npackets_vop;
    long	vp_close;
    //<MZ040108> adopt slice encoding
    long	vp_mb_num;
#ifdef iVCP1_VLC_HM
    long    slice_end;
#endif
#ifdef iVCP1E_HM_SPEC
    long    mb_skip_run;
#endif

    // Status Output
    long	intra_mbs;
    long	pic_intra_mbs;
    long    pic_skip_mbs;
    long	intraq_mbs;
    long	acpred_mbs;
    long	inter_mbs;
    long	interq_mbs;
    long	notcoded_mbs;
    long	forward_mbs;
    long	backward_mbs;
    long	interpolate_mbs;
    long	stuffing_mbs;
    long	field_dct_mbs;
    long	field_pred_mbs;
    long	annex1;
    long	mv_bits;
    long	tcoef_bits;
    //miche
    long	intra16x16_mbs;
    long	intra16x16_type[4];
    long	intra_chroma_type[4];
    long	intra4x4_mbs;
    long	intra4x4_type[9];
    long	err_code;
} m4vac_vlc_output;

typedef struct{
    //<MZ040922> for mb_out
    long	mb_type;
    long	cbp;
    long	intra_dc[6];
    long	pmv_h[4];
    long	pmv_v[4];
    long	mv_h[4];
    long	mv_v[4];
    long	total_coef[35];     //16YAC + 16CAC + 1YDC + 2CDC
    long	trailing_ones[35];  //16YAC + 16CAC + 1YDC + 2CDC
    long	ftop_ref;
    long	fbot_ref;
    long	btop_ref;
    long	bbot_ref;
    long	cause_close;
} m4vac_vlc_mbinfo;

//top level
long m4vac_vlc(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output);
int m4vac_vop_header(m4vac_vlc_input *vlc_input);
int m4vac_next_start_code();
int m4vac_interlaced_info(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output,
                          int mb_type, int cbp);
int m4vac_vlc_mb_in_ivop(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output,
                         int dquant);
int m4vac_vlc_mb_in_pvop(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output,
                         int dquant);
int m4vac_vlc_mb_in_bvop(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output,
                         int dquant);
int m4vac_vlc_mcbpc(int vop_type, int mb_type, int cbp);
int m4vac_vlc_cbpy(int intra_mb, int cbp);
int m4vac_vlc_dquant(int dquant);
int m4vac_vlc_dbquant(int dbquant);
int m4vac_vlc_raw_output(m4vac_vlc_input *vlc_input);
//<MZ041027> adopt endian of mbout
unsigned long m4vac_vlc_swap_dp(unsigned long lword, int endian);
//intra mb
int m4vac_vlc_intramb(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output);
int m4vac_vlc_intradc_blk(long *coef_in, int blknum);
int m4vac_vlc_intrablk(long *coef_in, int coef_number);
int m4vac_getdc_size(long dc_val);
int m4vac_intradc_luminance(long dc_value);
int m4vac_intradc_chrominance(long dc_value);
int m4vac_intraac_last0(int run, int level);
int m4vac_intraac_last0_lmax(int run);
int m4vac_intraac_last0_rmax(int abslevel);
int m4vac_intraac_last0_vlc(int run, int level);
int m4vac_intraac_last1(int run, int level);
int m4vac_intraac_last1_lmax(int run);
int m4vac_intraac_last1_rmax(int abslevel);
int m4vac_intraac_last1_vlc(int run, int level);
int m4vac_tcoef_flc(int last, int run, int level);
//inter mb
int m4vac_vlc_intermb(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output);
//void m4vac_pmv_calc(int mv_h, int mv_v, int *mvp_h, int *mvp_v,
//	int mb_posx, int mb_posy, int vp_enable, int vp_mb_num, int mb_sizex);
void m4vac_pmv_calc4(int *mv_h, int *mv_v, int *mvp_h, int *mvp_v,
                     int mb_posx, int mb_posy, int vp_enable, int vp_mb_num, int mb_sizex, int inter4v, int h263_pmv);
int m4vac_div2round(int mv_top, int mv_bot);
int m4vac_vlc_mv_in_pvop(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output,
                         int update_only);
int m4vac_vlc_mv_in_bvop(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output,
                         int update_only);
void m4vac_vlc_mv_range(int *mvd_h, int *mvd_v, int fcode);
int m4vac_vlc_mv(int mvd_h, int mvd_d, int fcode);
short m4vac_median_mv(short mv1, short mv2, short mv3, int h263_pmv);
//short m4vac_median_mv(short mv1, short mv2, short mv3);
//int m4vac_median_mv(int mv1, int mv2, int mv3);
int m4vac_vlc_mvdata(int mvdata);
int m4vac_vlc_interblk(long *coef_in, int coef_number);
int m4vac_interac_last0(int run, int level);
int m4vac_interac_last0_lmax(int run);
int m4vac_interac_last0_rmax(int abslevel);
int m4vac_interac_last0_vlc(int run, int level);
int m4vac_interac_last1(int run, int level);
int m4vac_interac_last1_lmax(int run);
int m4vac_interac_last1_rmax(int abslevel);
int m4vac_interac_last1_vlc(int run, int level);
//short header
int m4vac_vp_short_header(m4vac_vlc_input *vlc_input, int *mb_in_gob);
int m4vac_vp_short_header_end(int hext_enable);
int m4vac_vlc_gob_header(m4vac_vlc_input *vlc_input, int gob_number);
int m4vac_vlc_blk_sh(long *coef_in, int coef_number, int intra, int annex_t);
int m4vac_ac_last0_sh(int run, int level, int annex_t);
int m4vac_ac_last1_sh(int run, int level, int annex_t);
int m4vac_tcoef_flc_sh(int last, int run, int level, int annex_t);
int m4vac_byte_align_sh();
//h263profile3
int m4vac_h263_plus_header(m4vac_vlc_input *vlc_input, int *mb_in_gob);
int m4vac_vlc_slice_header_sh(m4vac_vlc_input *vlc_input);
int m4vac_vlc_modified_dquant(int mb_quant, int dquant);
int m4vac_vlc_intrablk_sh(long *coef_in, int coef_number, int annex_t);
int m4vac_intraac_last0_sh(int run, int level, int annex_t);
int m4vac_intraac_last1_sh(int run, int level, int annex_t);
int m4vac_intraac_last0_lmax_sh(int run);
int m4vac_intraac_last1_lmax_sh(int run);
int m4vac_intraac_last0_vlc_sh(int run, int level);
int m4vac_intraac_last1_vlc_sh(int run, int level);
//video packet & data partition
int m4vac_vlc_vp_header(m4vac_vlc_input *vlc_input);
int m4vac_mb_num_len(int mb_max);
int m4vac_vlc_mb_in_ivop_dp(m4vac_vlc_input *vlc_input,
                            m4vac_vlc_output *vlc_output, int dquant);
int m4vac_vlc_mb_in_pvop_dp(m4vac_vlc_input *vlc_input,
                            m4vac_vlc_output *vlc_output, int dquant);
//reversible vlc
int m4vac_vlc_intrablk_rv(long *coef_in, int coef_number);
int m4vac_vlc_interblk_rv(long *coef_in, int coef_number);
int m4vac_intraac_last0_rv(int run, int level);
int m4vac_interac_last0_rv(int run, int level);
int m4vac_ac_last1_rv(int run, int level);
int m4vac_intraac_last0_lmax_rv(int run);
int m4vac_interac_last0_lmax_rv(int run);
int m4vac_ac_last1_lmax_rv(int run);
int m4vac_intraac_last0_vlc_rv(int run, int level);
int m4vac_interac_last0_vlc_rv(int run, int level);
int m4vac_ac_last1_vlc_rv(int run, int level);
int m4vac_tcoef_flc_rv(int last, int run, int level);
//bottom level
//<IM031225>
//int bufreset(long *stream);
//<AK040123> for STREAM_RW
//int bufreset(long *stream, int bit_offset);
int bufreset(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output);
int bufflush();
int getoffset();
int concatdp();
int dpdisp();
int dpbits();
int m4vac_dp2_close(int vop_type);
int m4vac_change_partition(int part);
int putbits(unsigned int value, int length);
void m4vac_vlc_scan(m4vac_vlc_input *vlc_input);	// 03/02/17 added
int m4vac_scan_blk(long *coef_buf, long *coef_out, int direc);	// 03/09/08 add

//miche
int m4vac_vlc_reset(m4vac_vlc_output *vlc_output);
int m4vac_slice_header(m4vac_vlc_input *vlc_input);
int m4vac_slice_trailing();
#ifdef iVCP1_VLC_HM
int m4vac_get_offset();
int m4vac_filler(int size);
#endif
//<MZ040108> add I_PCM
int m4vac_alignmrnt_zero();
//int m4vac_vlc_mb_in_ivop_avc(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output, int dquant);
//int m4vac_vlc_mb_in_pvop_avc(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output, int dquant);
//<MZ040729> change slice end flow (fix)
//int m4vac_vlc_mb_in_ipslice_avc(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output, int dquant);
int m4vac_vlc_mb_in_ipslice_avc(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output, int dquant, int slice_over_mb_cnt);
//<MZ040209> fix multi slice encoding
//int m4vac_residual(long *coef_in, int intra16x16, int cbpy, int cbpc,
//					int mb_posx, int mb_posy, int update_only);
int m4vac_residual(long *coef_in, int intra16x16, int cbpy, int cbpc,
                   int mb_posx, int mb_posy, int mb_sizex, int vp_enable, int vp_mb_num, int is_field, int chroma_format_idc, int update_only);
int m4vac_residual_block(long *coef_buf, int start, int end, int nC, int *total_coef_out, int *trailing_ones_out);
void m4vac_scan_4x4(long *coef_buf, long *coef_out, int is_field);
#ifdef iVCP1_VLC_HM
void m4vac_scan_8x8_cavlc(long *coef_buf, long *coef_out, int is_field);
void m4vac_scan_2x4(long *coef_buf, long *coef_out);
#endif
int put_coeff_token(int trailing_ones, int total_coeff, int nC);
int put_level_prefix(int level_prefix);
int put_total_zeros(int total_zeros, int total_coeff);
int put_total_zeros_chromaDC(int total_zeros, int total_coeff);
int put_total_zeros_chromaDC422(int total_zeros, int total_coeff);
int put_run_before(int run_before, int zeros_left);
int put_ue(int codenum);
int put_se(int codenum);
int put_me(int coded_block_pattern, int intra_mb, int chroma_format_idc);

//<MZ031215> add P_8x8
//int m4vac_vlc_mv_avc(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output, int update_only);
//void m4vac_pmv_calc_avc(int *mv_h, int *mv_v, int *refidx, int *mvp_h, int *mvp_v,
//	int mb_posx, int mb_posy, int vp_enable, int vp_mb_num, int mb_sizex, int inter4v);
int m4vac_vlc_mv_avc(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output, int mb_type, int update_only);
//<MZ040107> improve P_skip
//void m4vac_pmv_calc_avc(int mv[4][2], int refidx[4], int mvp[4][2],
//	int mb_posx, int mb_posy, int vp_enable, int vp_mb_num, int mb_sizex, int mb_type);
void m4vac_pmv_calc_avc(int mv[4][2], int refidx[4], int mvp[4][2],
                        int mb_posx, int mb_posy, int vp_enable, int vp_mb_num, int mb_sizex, int mb_type,
                        int judge_p_skip, int *jps_mba_rf, int *jps_mbb_rf);
short m4vac_pred_mv_avc(short mva, short mvb, short mvc, int refa, int refb, int refc, int ref);
short m4vac_median_mv_avc(short mv1, short mv2, short mv3);
int m4vac_vlc_intra4x4(m4vac_vlc_input *vlc_input, m4vac_vlc_output *vlc_output, int update_only);
int m4vac_vlc_pred_i4x4(int nA, int nB);
int m4vac_vlc_nC(int nA, int nB);

    // end vlc.h
/*--------------------------------------------------------------------------*/
/* SH73xx MPEG-4 Encoder Module Ver.1.0                                     */
/*	Copyright (C) Renesas Technology Corp., 2003. All rights reserved.		*/
/*--------------------------------------------------------------------------*/
/*  m4vae_bitstream.c :                                                     */
/*--------------------------------------------------------------------------*/
/*  1.000  2002/10/01  start codes                                          */
/*  1.000  2002/10/01                                                       */
/*--------------------------------------------------------------------------*/
#include "mpeg4venc.h"

static long strmBuff;	// 32bit buffer
static long offsetInDWord = 0;
#ifdef iVCP1E_HM_SPEC
long hd_ems_ins_flag = 0;
static unsigned long prev_buf = 0xffff;
#endif

// Array to be used for masking the values to be put into the buffer
static const long BitMask[32] = {
    0x00000000, 0x80000000, 0xc0000000, 0xe0000000, 
    0xf0000000, 0xf8000000, 0xfc000000, 0xfe000000, 
    0xff000000, 0xff800000, 0xffc00000, 0xffe00000, 
    0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000, 
    0xffff0000, 0xffff8000, 0xffffc000, 0xffffe000, 
    0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00, 
    0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0, 
    0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe
};

int m4vae_em_ins(unsigned long ibuf, long *len, unsigned char *obuf) {
    long i = 0;
    long olen = *len;

    obuf[0] = (ibuf >> 0) & 0xff;
    obuf[1] = (ibuf >> 8) & 0xff;
    obuf[2] = (ibuf >> 16) & 0xff;
    obuf[3] = (ibuf >> 24) & 0xff;

    for (i = *len; i < 6; i++) {
        obuf[i] = 0xff;
    }

    if (prev_buf == 0) {
        if (obuf[0] < 0x4) {
            for (i = 4; i > 0; i--) {
                obuf[i] = obuf[i - 1];
            }
            obuf[0] = 0x03;
            olen++;
            if (obuf[1] == 0 && obuf[2] == 0 && obuf[3] < 4) {
                for (i = 5; i > 3; i--) {
                    obuf[i] = obuf[i - 1];
                }
                obuf[3] = 0x03;
                olen++;
            }
            else if (obuf[2] == 0 && obuf[3] == 0 && obuf[4] < 4) {
                for (i = 5; i > 4; i--) {
                    obuf[i] = obuf[i - 1];
                }
                obuf[4] = 0x03;
                olen++;
            }
        }
        else if (obuf[1] == 0 && obuf[2] == 0 && obuf[3] < 4) {
            for (i = 5; i > 3; i--) {
                obuf[i] = obuf[i - 1];
            }
            obuf[3] = 0x03;
            olen++;
        }
    }
    else if ((prev_buf & 0xff) == 0) {
        if (obuf[0] == 0 && obuf[1] < 4) {
            for (i = 5; i > 1; i--) {
                obuf[i] = obuf[i - 1];
            }
            obuf[1] = 0x03;
            olen++;
            if (obuf[2] == 0 && obuf[3] == 0 && obuf[4] < 4) {
                for (i = 5; i > 4; i--) {
                    obuf[i] = obuf[i - 1];
                }
                obuf[4] = 0x03;
                olen++;
            }
        }
        else if (obuf[1] == 0 && obuf[2] == 0 && obuf[3] < 4) {
            for (i = 5; i > 3; i--) {
                obuf[i] = obuf[i - 1];
            }
            obuf[3] = 0x03;
            olen++;
        }
    }
    else if (obuf[0] == 0 && obuf[1] == 0 && obuf[2] < 4) {
        for (i = 5; i > 2; i--) {
            obuf[i] = obuf[i - 1];
        }
        obuf[2] = 0x03;
        olen++;
    }
    else if (obuf[1] == 0 && obuf[2] == 0 && obuf[3] < 4) {
        for (i = 5; i > 3; i--) {
            obuf[i] = obuf[i - 1];
        }
        obuf[3] = 0x03;
        olen++;
    }
    *len = olen;
    return 1;
}

//<IM031225>
int m4vae_putbits ( unsigned long  value, long length )
{
    long temp0;
    long temp1;
    unsigned long *buff;

    buff = (unsigned long *)&strmBuff;
    *buff &= BitMask[offsetInDWord];

    temp0 = offsetInDWord + length;
    temp1 = 32 - temp0;
    if ( temp1 > 0 )
    {
        *buff |= (value << (temp1));
        offsetInDWord = temp0;
    }
    else
    {
        temp1 =  - temp1; /* temp1 > 0 */
        *buff |= (value >> temp1);
#ifndef _BIG
        {
            unsigned long tmp = strmBuff;
            *buff =  ((tmp & 0xFF000000) >> 24);
            *buff |= ((tmp & 0x00FF0000) >> 8);
            *buff |= ((tmp & 0x0000FF00) << 8);
            *buff |= ((tmp & 0x000000FF) << 24);
        }
#endif
        if (hd_ems_ins_flag == 1) {
            unsigned char ems_buf[6] = { 0 };
            long len = 4;
            m4vae_em_ins(*buff, &len, ems_buf);
            prev_buf = ((ems_buf[len-2] & 0xff) << 8) | (ems_buf[len-1] & 0xff);
            m4vae_output_byte((char*)ems_buf, len);	// write Len byte
            length += (len - 4)*8;
        }
        else {
            m4vae_output_byte((char*)buff, 4);	// write 4 byte
            prev_buf = ((*buff) >> 16) & 0xffff;
        }
        *buff = (value << (32 - temp1)); /* (32 - temp2) = (64 - length - offsetInDWord) */
        offsetInDWord = temp1;
    }      

    return length;
}


//<IM040206>
int m4vae_putstuffbitsmpeg4(long h264)
{
    int length = 0;
    if (offsetInDWord == 0) {
        strmBuff = 0;
    }
    strmBuff >>= (32 - offsetInDWord); 
    strmBuff <<= (32 - offsetInDWord); 

    if (h264){
        strmBuff |= 1 << ((32 - offsetInDWord) - 1);
    }else{
        strmBuff |= ( (1 << ((32 - offsetInDWord) - 1)) - 1 );
    }

    offsetInDWord = ((offsetInDWord + 8) / 8) * 8; 

    if(offsetInDWord == 32)
    {
        offsetInDWord = 0;
#ifndef _BIG
        {
            unsigned long tmp = strmBuff;
            strmBuff =  ((tmp & 0xFF000000) >> 24);
            strmBuff |= ((tmp & 0x00FF0000) >> 8);
            strmBuff |= ((tmp & 0x0000FF00) << 8);
            strmBuff |= ((tmp & 0x000000FF) << 24);
        }
#endif

        if (hd_ems_ins_flag == 1) {
            unsigned char ems_buf[6] = { 0 };
            long len = 4;
            m4vae_em_ins(strmBuff, &len, ems_buf);
            prev_buf = ((ems_buf[len - 2] & 0xff) << 8) | (ems_buf[len - 1] & 0xff);
            m4vae_output_byte((char*)ems_buf, len);	// write Len byte
            length = (len - 4) * 8;
        }
        else {
            m4vae_output_byte((char*)&strmBuff, 4);	// write 4 byte
            prev_buf = (strmBuff >> 16) & 0xffff;
        }
    }

    return length;
}

int m4vae_flushbits(long h264)
{
    int length = 0;
    length = m4vae_putstuffbitsmpeg4(h264);

#ifndef _BIG
    {
        unsigned long tmp = strmBuff;
        strmBuff =  ((tmp & 0xFF000000) >> 24);
        strmBuff |= ((tmp & 0x00FF0000) >> 8);
        strmBuff |= ((tmp & 0x0000FF00) << 8);
        strmBuff |= ((tmp & 0x000000FF) << 24);
    }
#endif
    if (hd_ems_ins_flag == 1) {
        unsigned char ems_buf[6] = { 0 };
        long len;
        len = offsetInDWord >> 3;
        m4vae_em_ins(strmBuff, &len, ems_buf);
        prev_buf = ((ems_buf[len - 2] & 0xff) << 8) | (ems_buf[len - 1] & 0xff);
        m4vae_output_byte((char*)ems_buf, len);	// write Len byte
        length += (len - (offsetInDWord >> 3)) * 8;
    }
    else {
        m4vae_output_byte((char*)&strmBuff, (offsetInDWord >> 3));
        prev_buf = (strmBuff >> 16) & 0xffff;
    }
    offsetInDWord = 0;

    return length;
}


//static long buff;	// 32bit buffer
//
//long m4vae_putbits(long code,long length)
//{
//    long out_code = 0;
//    long count = 8;
//    long shift;
//
//    code &= (1<<length) - 1;
//    buff += length;
//    while ( length > 0 ) {
//        shift = (length - count > 0) ? count : length ;
//        out_code = ((out_code << shift) | (code >> (length - shift))) & 255 ;
//        length -= shift ;
//        count  -= shift ;
//        if(count == 0){
//            count = 8 ;
//            m4vae_output_byte((char*)&out_code, 1);
//        }
//    }
//    return(count) ;
//}
//
//void m4vae_putstuffbitsmpeg4(void)
//{
//
//}
// == end bitstream.c

/*--------------------------------------------------------------------------*/
/* SH73xx MPEG-4 Encoder Module Ver.1.0                                     */
/*	Copyright (C) Renesas Technology Corp., 2003. All rights reserved.	*/
/*--------------------------------------------------------------------------*/
/*	mpeg4main.c : 						                					*/
/*--------------------------------------------------------------------------*/
/*  1.000  2002/10/01  start codes                                          */
/*  1.000  2002/10/01                                                       */
/*--------------------------------------------------------------------------*/


/** ƒvƒƒgƒ^ƒCƒv **/
long m4vae_putbits(long code,long length);
int  m4vae_putstuffbitsmpeg4(long h264);
int  m4vae_flushbits(long h264);

// == end bitstream.h
    
