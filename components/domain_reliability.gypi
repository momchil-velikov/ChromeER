# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      # GN version: //components/domain_reliability
      'target_name': 'domain_reliability',
      'type': '<(component)',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_prefs',
        '../components/components.gyp:keyed_service_core',
        '../content/content.gyp:content_browser',
        '../net/net.gyp:net',
        '../url/url.gyp:url_lib',
      ],
      'include_dirs': [
        '..',
      ],
      'defines': [
        'DOMAIN_RELIABILITY_IMPLEMENTATION',
      ],
      'sources': [
        # Note: sources list duplicated in GN build.
        'domain_reliability/baked_in_configs.h',
        'domain_reliability/beacon.cc',
        'domain_reliability/beacon.h',
        'domain_reliability/clear_mode.h',
        'domain_reliability/config.cc',
        'domain_reliability/config.h',
        'domain_reliability/context.cc',
        'domain_reliability/context.h',
        'domain_reliability/dispatcher.cc',
        'domain_reliability/dispatcher.h',
        'domain_reliability/domain_reliability_export.h',
        'domain_reliability/monitor.cc',
        'domain_reliability/monitor.h',
        'domain_reliability/scheduler.cc',
        'domain_reliability/scheduler.h',
        'domain_reliability/service.cc',
        'domain_reliability/service.h',
        'domain_reliability/uploader.cc',
        'domain_reliability/uploader.h',
        'domain_reliability/util.cc',
        'domain_reliability/util.h',
      ],
      'actions': [
        {
          'action_name': 'bake_in_configs',
          'variables': {
            'bake_in_configs_script': 'domain_reliability/bake_in_configs.py',
            'baked_in_configs_cc':
                '<(INTERMEDIATE_DIR)/domain_reliability/baked_in_configs.cc',
            'baked_in_configs': [
              'domain_reliability/baked_in_configs/accounts_google_com.json',
              'domain_reliability/baked_in_configs/ad_doubleclick_net.json',
              'domain_reliability/baked_in_configs/apis_google_com.json',
              'domain_reliability/baked_in_configs/c_admob_com.json',
              'domain_reliability/baked_in_configs/clients2_google_com.json',
              'domain_reliability/baked_in_configs/csi_gstatic_com.json',
              'domain_reliability/baked_in_configs/ddm_google_com.json',
              'domain_reliability/baked_in_configs/docs_google_com.json',
              'domain_reliability/baked_in_configs/drive_google_com.json',
              'domain_reliability/baked_in_configs/e_admob_com.json',
              'domain_reliability/baked_in_configs/fonts_googleapis_com.json',
              'domain_reliability/baked_in_configs/googleads4_g_doubleclick_net.json',
              'domain_reliability/baked_in_configs/googleads_g_doubleclick_net.json',
              'domain_reliability/baked_in_configs/gstatic_com.json',
              'domain_reliability/baked_in_configs/lh3_ggpht_com.json',
              'domain_reliability/baked_in_configs/lh4_ggpht_com.json',
              'domain_reliability/baked_in_configs/lh5_ggpht_com.json',
              'domain_reliability/baked_in_configs/lh6_ggpht_com.json',
              'domain_reliability/baked_in_configs/mail_google_com.json',
              'domain_reliability/baked_in_configs/media_admob_com.json',
              'domain_reliability/baked_in_configs/pagead2_googlesyndication_com.json',
              'domain_reliability/baked_in_configs/partner_googleadservices_com.json',
              'domain_reliability/baked_in_configs/pubads_g_doubleclick_net.json',
              'domain_reliability/baked_in_configs/redirector_googlevideo_com.json',
              'domain_reliability/baked_in_configs/s0_2mdn_net.json',
              'domain_reliability/baked_in_configs/ssl_gstatic_com.json',
              'domain_reliability/baked_in_configs/star_2mdn_net.json',
              'domain_reliability/baked_in_configs/star_admob_biz.json',
              'domain_reliability/baked_in_configs/star_admob_co_in.json',
              'domain_reliability/baked_in_configs/star_admob_co_kr.json',
              'domain_reliability/baked_in_configs/star_admob_com_br.json',
              'domain_reliability/baked_in_configs/star_admob_com_es.json',
              'domain_reliability/baked_in_configs/star_admob_com_fr.json',
              'domain_reliability/baked_in_configs/star_admob_com.json',
              'domain_reliability/baked_in_configs/star_admob_com_mx.json',
              'domain_reliability/baked_in_configs/star_admob_com_pt.json',
              'domain_reliability/baked_in_configs/star_admob_co_nz.json',
              'domain_reliability/baked_in_configs/star_admob_co_uk.json',
              'domain_reliability/baked_in_configs/star_admob_co_za.json',
              'domain_reliability/baked_in_configs/star_admob_de.json',
              'domain_reliability/baked_in_configs/star_admob_dk.json',
              'domain_reliability/baked_in_configs/star_admob_es.json',
              'domain_reliability/baked_in_configs/star_admob_fi.json',
              'domain_reliability/baked_in_configs/star_admob_fr.json',
              'domain_reliability/baked_in_configs/star_admob_gr.json',
              'domain_reliability/baked_in_configs/star_admob_hk.json',
              'domain_reliability/baked_in_configs/star_admob_ie.json',
              'domain_reliability/baked_in_configs/star_admob_in.json',
              'domain_reliability/baked_in_configs/star_admob_it.json',
              'domain_reliability/baked_in_configs/star_admob_jp.json',
              'domain_reliability/baked_in_configs/star_admob_kr.json',
              'domain_reliability/baked_in_configs/star_admob_mobi.json',
              'domain_reliability/baked_in_configs/star_admob_no.json',
              'domain_reliability/baked_in_configs/star_admob_ph.json',
              'domain_reliability/baked_in_configs/star_admob_pt.json',
              'domain_reliability/baked_in_configs/star_admob_sg.json',
              'domain_reliability/baked_in_configs/star_admob_tw.json',
              'domain_reliability/baked_in_configs/star_admob_us.json',
              'domain_reliability/baked_in_configs/star_admob_vn.json',
              'domain_reliability/baked_in_configs/star_c_2mdn_net.json',
              'domain_reliability/baked_in_configs/star_c_android_clients_google_com.json',
              'domain_reliability/baked_in_configs/star_c_bigcache_googleapis_com.json',
              'domain_reliability/baked_in_configs/star_c_doc-0-0-sj_sj_googleusercontent_com.json',
              'domain_reliability/baked_in_configs/star_c_docs_google_com.json',
              'domain_reliability/baked_in_configs/star_c_drive_google_com.json',
              'domain_reliability/baked_in_configs/star_c_googlesyndication_com.json',
              'domain_reliability/baked_in_configs/star_c_pack_google_com.json',
              'domain_reliability/baked_in_configs/star_c_play_google_com.json',
              'domain_reliability/baked_in_configs/star_c_youtube_com.json',
              'domain_reliability/baked_in_configs/star_dartmotif_com.json',
              'domain_reliability/baked_in_configs/star_doubleclick_com.json',
              'domain_reliability/baked_in_configs/star_doubleclick_ne_jp.json',
              'domain_reliability/baked_in_configs/star_doubleclick_net.json',
              'domain_reliability/baked_in_configs/star_doubleclickusercontent_com.json',
              'domain_reliability/baked_in_configs/star_g_co.json',
              'domain_reliability/baked_in_configs/star_g_doubleclick_net.json',
              'domain_reliability/baked_in_configs/star_ggpht_com.json',
              'domain_reliability/baked_in_configs/star_gmodules_com.json',
              'domain_reliability/baked_in_configs/star_google_ac.json',
              'domain_reliability/baked_in_configs/star_google_ad.json',
              'domain_reliability/baked_in_configs/star_googleadservices_com.json',
              'domain_reliability/baked_in_configs/star_google_ae.json',
              'domain_reliability/baked_in_configs/star_google_af.json',
              'domain_reliability/baked_in_configs/star_google_ag.json',
              'domain_reliability/baked_in_configs/star_google_al.json',
              'domain_reliability/baked_in_configs/star_googlealumni_com.json',
              'domain_reliability/baked_in_configs/star_google_am.json',
              'domain_reliability/baked_in_configs/star_google-analytics_com.json',
              'domain_reliability/baked_in_configs/star_googleapis_com.json',
              'domain_reliability/baked_in_configs/star_googleapps_com.json',
              'domain_reliability/baked_in_configs/star_google_as.json',
              'domain_reliability/baked_in_configs/star_google_at.json',
              'domain_reliability/baked_in_configs/star_google_az.json',
              'domain_reliability/baked_in_configs/star_google_ba.json',
              'domain_reliability/baked_in_configs/star_google_be.json',
              'domain_reliability/baked_in_configs/star_google_bf.json',
              'domain_reliability/baked_in_configs/star_google_bg.json',
              'domain_reliability/baked_in_configs/star_google_bi.json',
              'domain_reliability/baked_in_configs/star_google_bj.json',
              'domain_reliability/baked_in_configs/star_google_bs.json',
              'domain_reliability/baked_in_configs/star_google_bt.json',
              'domain_reliability/baked_in_configs/star_google_by.json',
              'domain_reliability/baked_in_configs/star_google_ca.json',
              'domain_reliability/baked_in_configs/star_google_cat.json',
              'domain_reliability/baked_in_configs/star_googlecbs_com.json',
              'domain_reliability/baked_in_configs/star_google_cc.json',
              'domain_reliability/baked_in_configs/star_google_cd.json',
              'domain_reliability/baked_in_configs/star_google_cf.json',
              'domain_reliability/baked_in_configs/star_google_cg.json',
              'domain_reliability/baked_in_configs/star_google_ch.json',
              'domain_reliability/baked_in_configs/star_google_ci.json',
              'domain_reliability/baked_in_configs/star_google_cl.json',
              'domain_reliability/baked_in_configs/star_google_cm.json',
              'domain_reliability/baked_in_configs/star_google_cn.json',
              'domain_reliability/baked_in_configs/star_google_co_ao.json',
              'domain_reliability/baked_in_configs/star_google_co_bw.json',
              'domain_reliability/baked_in_configs/star_google_co_ck.json',
              'domain_reliability/baked_in_configs/star_google_co_cr.json',
              'domain_reliability/baked_in_configs/star_google_co_hu.json',
              'domain_reliability/baked_in_configs/star_google_co_id.json',
              'domain_reliability/baked_in_configs/star_google_co_il.json',
              'domain_reliability/baked_in_configs/star_google_co_im.json',
              'domain_reliability/baked_in_configs/star_google_co_in.json',
              'domain_reliability/baked_in_configs/star_google_co_je.json',
              'domain_reliability/baked_in_configs/star_google_co_jp.json',
              'domain_reliability/baked_in_configs/star_google_co_ke.json',
              'domain_reliability/baked_in_configs/star_google_co_kr.json',
              'domain_reliability/baked_in_configs/star_google_co_ls.json',
              'domain_reliability/baked_in_configs/star_google_com_af.json',
              'domain_reliability/baked_in_configs/star_google_com_ag.json',
              'domain_reliability/baked_in_configs/star_google_com_ai.json',
              'domain_reliability/baked_in_configs/star_google_co_ma.json',
              'domain_reliability/baked_in_configs/star_google_com_ar.json',
              'domain_reliability/baked_in_configs/star_google_com_au.json',
              'domain_reliability/baked_in_configs/star_google_com_bd.json',
              'domain_reliability/baked_in_configs/star_google_com_bh.json',
              'domain_reliability/baked_in_configs/star_google_com_bn.json',
              'domain_reliability/baked_in_configs/star_google_com_bo.json',
              'domain_reliability/baked_in_configs/star_google_com_br.json',
              'domain_reliability/baked_in_configs/star_google_com_by.json',
              'domain_reliability/baked_in_configs/star_google_com_bz.json',
              'domain_reliability/baked_in_configs/star_google_com_cn.json',
              'domain_reliability/baked_in_configs/star_google_com_co.json',
              'domain_reliability/baked_in_configs/star_google_com_cu.json',
              'domain_reliability/baked_in_configs/star_google_com_cy.json',
              'domain_reliability/baked_in_configs/star_google_com_do.json',
              'domain_reliability/baked_in_configs/star_google_com_ec.json',
              'domain_reliability/baked_in_configs/star_google_com_eg.json',
              'domain_reliability/baked_in_configs/star_google_com_et.json',
              'domain_reliability/baked_in_configs/star_google_com_fj.json',
              'domain_reliability/baked_in_configs/star_google_com_ge.json',
              'domain_reliability/baked_in_configs/star_google_com_gh.json',
              'domain_reliability/baked_in_configs/star_google_com_gi.json',
              'domain_reliability/baked_in_configs/star_google_com_gr.json',
              'domain_reliability/baked_in_configs/star_google_com_gt.json',
              'domain_reliability/baked_in_configs/star_google_com_hk.json',
              'domain_reliability/baked_in_configs/star_google_com_iq.json',
              'domain_reliability/baked_in_configs/star_google_com_jm.json',
              'domain_reliability/baked_in_configs/star_google_com_jo.json',
              'domain_reliability/baked_in_configs/star_google_com.json',
              'domain_reliability/baked_in_configs/star_google_com_kh.json',
              'domain_reliability/baked_in_configs/star_google_com_kw.json',
              'domain_reliability/baked_in_configs/star_google_com_lb.json',
              'domain_reliability/baked_in_configs/star_google_com_ly.json',
              'domain_reliability/baked_in_configs/star_googlecommerce_com.json',
              'domain_reliability/baked_in_configs/star_google_com_mm.json',
              'domain_reliability/baked_in_configs/star_google_com_mt.json',
              'domain_reliability/baked_in_configs/star_google_com_mx.json',
              'domain_reliability/baked_in_configs/star_google_com_my.json',
              'domain_reliability/baked_in_configs/star_google_com_na.json',
              'domain_reliability/baked_in_configs/star_google_com_nf.json',
              'domain_reliability/baked_in_configs/star_google_com_ng.json',
              'domain_reliability/baked_in_configs/star_google_com_ni.json',
              'domain_reliability/baked_in_configs/star_google_com_np.json',
              'domain_reliability/baked_in_configs/star_google_com_nr.json',
              'domain_reliability/baked_in_configs/star_google_com_om.json',
              'domain_reliability/baked_in_configs/star_google_com_pa.json',
              'domain_reliability/baked_in_configs/star_google_com_pe.json',
              'domain_reliability/baked_in_configs/star_google_com_pg.json',
              'domain_reliability/baked_in_configs/star_google_com_ph.json',
              'domain_reliability/baked_in_configs/star_google_com_pk.json',
              'domain_reliability/baked_in_configs/star_google_com_pl.json',
              'domain_reliability/baked_in_configs/star_google_com_pr.json',
              'domain_reliability/baked_in_configs/star_google_com_py.json',
              'domain_reliability/baked_in_configs/star_google_com_qa.json',
              'domain_reliability/baked_in_configs/star_google_com_ru.json',
              'domain_reliability/baked_in_configs/star_google_com_sa.json',
              'domain_reliability/baked_in_configs/star_google_com_sb.json',
              'domain_reliability/baked_in_configs/star_google_com_sg.json',
              'domain_reliability/baked_in_configs/star_google_com_sl.json',
              'domain_reliability/baked_in_configs/star_google_com_sv.json',
              'domain_reliability/baked_in_configs/star_google_com_tj.json',
              'domain_reliability/baked_in_configs/star_google_com_tn.json',
              'domain_reliability/baked_in_configs/star_google_com_tr.json',
              'domain_reliability/baked_in_configs/star_google_com_tw.json',
              'domain_reliability/baked_in_configs/star_google_com_ua.json',
              'domain_reliability/baked_in_configs/star_google_com_uy.json',
              'domain_reliability/baked_in_configs/star_google_com_vc.json',
              'domain_reliability/baked_in_configs/star_google_com_ve.json',
              'domain_reliability/baked_in_configs/star_google_com_vn.json',
              'domain_reliability/baked_in_configs/star_google_co_mz.json',
              'domain_reliability/baked_in_configs/star_google_co_nz.json',
              'domain_reliability/baked_in_configs/star_google_co_th.json',
              'domain_reliability/baked_in_configs/star_google_co_tz.json',
              'domain_reliability/baked_in_configs/star_google_co_ug.json',
              'domain_reliability/baked_in_configs/star_google_co_uk.json',
              'domain_reliability/baked_in_configs/star_google_co_uz.json',
              'domain_reliability/baked_in_configs/star_google_co_ve.json',
              'domain_reliability/baked_in_configs/star_google_co_vi.json',
              'domain_reliability/baked_in_configs/star_google_co_za.json',
              'domain_reliability/baked_in_configs/star_google_co_zm.json',
              'domain_reliability/baked_in_configs/star_google_co_zw.json',
              'domain_reliability/baked_in_configs/star_google_cv.json',
              'domain_reliability/baked_in_configs/star_google_cz.json',
              'domain_reliability/baked_in_configs/star_google_de.json',
              'domain_reliability/baked_in_configs/star_google_dj.json',
              'domain_reliability/baked_in_configs/star_google_dk.json',
              'domain_reliability/baked_in_configs/star_google_dm.json',
              'domain_reliability/baked_in_configs/star_googledrive_com.json',
              'domain_reliability/baked_in_configs/star_google_dz.json',
              'domain_reliability/baked_in_configs/star_google_ee.json',
              'domain_reliability/baked_in_configs/star_googleenterprise_com.json',
              'domain_reliability/baked_in_configs/star_google_es.json',
              'domain_reliability/baked_in_configs/star_google_fi.json',
              'domain_reliability/baked_in_configs/star_google_fm.json',
              'domain_reliability/baked_in_configs/star_google_fr.json',
              'domain_reliability/baked_in_configs/star_google_ga.json',
              'domain_reliability/baked_in_configs/star_google_ge.json',
              'domain_reliability/baked_in_configs/star_google_gg.json',
              'domain_reliability/baked_in_configs/star_google_gl.json',
              'domain_reliability/baked_in_configs/star_google_gm.json',
              'domain_reliability/baked_in_configs/star_googlegoro_com.json',
              'domain_reliability/baked_in_configs/star_google_gp.json',
              'domain_reliability/baked_in_configs/star_google_gr.json',
              'domain_reliability/baked_in_configs/star_google_gy.json',
              'domain_reliability/baked_in_configs/star_google_hk.json',
              'domain_reliability/baked_in_configs/star_google_hn.json',
              'domain_reliability/baked_in_configs/star_googlehosted_com.json',
              'domain_reliability/baked_in_configs/star_google_hr.json',
              'domain_reliability/baked_in_configs/star_google_ht.json',
              'domain_reliability/baked_in_configs/star_google_hu.json',
              'domain_reliability/baked_in_configs/star_google_ie.json',
              'domain_reliability/baked_in_configs/star_google_im.json',
              'domain_reliability/baked_in_configs/star_google_info.json',
              'domain_reliability/baked_in_configs/star_google_iq.json',
              'domain_reliability/baked_in_configs/star_google_ir.json',
              'domain_reliability/baked_in_configs/star_google_is.json',
              'domain_reliability/baked_in_configs/star_google_it_ao.json',
              'domain_reliability/baked_in_configs/star_google_it.json',
              'domain_reliability/baked_in_configs/star_google_je.json',
              'domain_reliability/baked_in_configs/star_google_jobs.json',
              'domain_reliability/baked_in_configs/star_google_jo.json',
              'domain_reliability/baked_in_configs/star_google_jp.json',
              'domain_reliability/baked_in_configs/star_google_kg.json',
              'domain_reliability/baked_in_configs/star_google_ki.json',
              'domain_reliability/baked_in_configs/star_google_kz.json',
              'domain_reliability/baked_in_configs/star_google_la.json',
              'domain_reliability/baked_in_configs/star_google_li.json',
              'domain_reliability/baked_in_configs/star_google_lk.json',
              'domain_reliability/baked_in_configs/star_google_lt.json',
              'domain_reliability/baked_in_configs/star_google_lu.json',
              'domain_reliability/baked_in_configs/star_google_lv.json',
              'domain_reliability/baked_in_configs/star_google_md.json',
              'domain_reliability/baked_in_configs/star_google_me.json',
              'domain_reliability/baked_in_configs/star_google_mg.json',
              'domain_reliability/baked_in_configs/star_google_mk.json',
              'domain_reliability/baked_in_configs/star_google_ml.json',
              'domain_reliability/baked_in_configs/star_google_mn.json',
              'domain_reliability/baked_in_configs/star_google_ms.json',
              'domain_reliability/baked_in_configs/star_google_mu.json',
              'domain_reliability/baked_in_configs/star_google_mv.json',
              'domain_reliability/baked_in_configs/star_google_mw.json',
              'domain_reliability/baked_in_configs/star_google_ne_jp.json',
              'domain_reliability/baked_in_configs/star_google_ne.json',
              'domain_reliability/baked_in_configs/star_google_net.json',
              'domain_reliability/baked_in_configs/star_google_ng.json',
              'domain_reliability/baked_in_configs/star_google_nl.json',
              'domain_reliability/baked_in_configs/star_google_no.json',
              'domain_reliability/baked_in_configs/star_google_nr.json',
              'domain_reliability/baked_in_configs/star_google_nu.json',
              'domain_reliability/baked_in_configs/star_google_off_ai.json',
              'domain_reliability/baked_in_configs/star_google_org.json',
              'domain_reliability/baked_in_configs/star_googlepayments_com.json',
              'domain_reliability/baked_in_configs/star_google_pk.json',
              'domain_reliability/baked_in_configs/star_google_pl.json',
              'domain_reliability/baked_in_configs/star_google_pn.json',
              'domain_reliability/baked_in_configs/star_google_ps.json',
              'domain_reliability/baked_in_configs/star_google_pt.json',
              'domain_reliability/baked_in_configs/star_google_ro.json',
              'domain_reliability/baked_in_configs/star_google_rs.json',
              'domain_reliability/baked_in_configs/star_google_ru.json',
              'domain_reliability/baked_in_configs/star_google_rw.json',
              'domain_reliability/baked_in_configs/star_google_sc.json',
              'domain_reliability/baked_in_configs/star_google_se.json',
              'domain_reliability/baked_in_configs/star_google_sh.json',
              'domain_reliability/baked_in_configs/star_google_si.json',
              'domain_reliability/baked_in_configs/star_google_sk.json',
              'domain_reliability/baked_in_configs/star_google_sm.json',
              'domain_reliability/baked_in_configs/star_google_sn.json',
              'domain_reliability/baked_in_configs/star_google_so.json',
              'domain_reliability/baked_in_configs/star_googlesource_com.json',
              'domain_reliability/baked_in_configs/star_google_sr.json',
              'domain_reliability/baked_in_configs/star_google_st.json',
              'domain_reliability/baked_in_configs/star_google-syndication_com.json',
              'domain_reliability/baked_in_configs/star_googlesyndication_com.json',
              'domain_reliability/baked_in_configs/star_googletagmanager_com.json',
              'domain_reliability/baked_in_configs/star_googletagservices_com.json',
              'domain_reliability/baked_in_configs/star_google_td.json',
              'domain_reliability/baked_in_configs/star_google_tg.json',
              'domain_reliability/baked_in_configs/star_google_tk.json',
              'domain_reliability/baked_in_configs/star_google_tl.json',
              'domain_reliability/baked_in_configs/star_google_tm.json',
              'domain_reliability/baked_in_configs/star_google_tn.json',
              'domain_reliability/baked_in_configs/star_google_to.json',
              'domain_reliability/baked_in_configs/star_google_tt.json',
              'domain_reliability/baked_in_configs/star_googleusercontent_com.json',
              'domain_reliability/baked_in_configs/star_google_us.json',
              'domain_reliability/baked_in_configs/star_google_uz.json',
              'domain_reliability/baked_in_configs/star_google_vg.json',
              'domain_reliability/baked_in_configs/star_googlevideo_com.json',
              'domain_reliability/baked_in_configs/star_google_vu.json',
              'domain_reliability/baked_in_configs/star_google_ws.json',
              'domain_reliability/baked_in_configs/star_goo_gl.json',
              'domain_reliability/baked_in_configs/star_gstatic_com.json',
              'domain_reliability/baked_in_configs/star_gvt1_com.json',
              'domain_reliability/baked_in_configs/star_gvt2_com.json',
              'domain_reliability/baked_in_configs/star_withgoogle_com.json',
              'domain_reliability/baked_in_configs/star_youtube-3rd-party_com.json',
              'domain_reliability/baked_in_configs/star_youtube_ae.json',
              'domain_reliability/baked_in_configs/star_youtube_al.json',
              'domain_reliability/baked_in_configs/star_youtube_am.json',
              'domain_reliability/baked_in_configs/star_youtube_at.json',
              'domain_reliability/baked_in_configs/star_youtube_az.json',
              'domain_reliability/baked_in_configs/star_youtube_ba.json',
              'domain_reliability/baked_in_configs/star_youtube_be.json',
              'domain_reliability/baked_in_configs/star_youtube_bg.json',
              'domain_reliability/baked_in_configs/star_youtube_bh.json',
              'domain_reliability/baked_in_configs/star_youtube_bo.json',
              'domain_reliability/baked_in_configs/star_youtube_ca.json',
              'domain_reliability/baked_in_configs/star_youtube_cat.json',
              'domain_reliability/baked_in_configs/star_youtube_ch.json',
              'domain_reliability/baked_in_configs/star_youtube_cl.json',
              'domain_reliability/baked_in_configs/star_youtube_co_ae.json',
              'domain_reliability/baked_in_configs/star_youtube_co_at.json',
              'domain_reliability/baked_in_configs/star_youtube_co_hu.json',
              'domain_reliability/baked_in_configs/star_youtube_co_id.json',
              'domain_reliability/baked_in_configs/star_youtube_co_il.json',
              'domain_reliability/baked_in_configs/star_youtube_co_in.json',
              'domain_reliability/baked_in_configs/star_youtube_co_jp.json',
              'domain_reliability/baked_in_configs/star_youtube_co.json',
              'domain_reliability/baked_in_configs/star_youtube_co_ke.json',
              'domain_reliability/baked_in_configs/star_youtube_co_kr.json',
              'domain_reliability/baked_in_configs/star_youtube_co_ma.json',
              'domain_reliability/baked_in_configs/star_youtube_com_ar.json',
              'domain_reliability/baked_in_configs/star_youtube_com_au.json',
              'domain_reliability/baked_in_configs/star_youtube_com_az.json',
              'domain_reliability/baked_in_configs/star_youtube_com_bh.json',
              'domain_reliability/baked_in_configs/star_youtube_com_bo.json',
              'domain_reliability/baked_in_configs/star_youtube_com_br.json',
              'domain_reliability/baked_in_configs/star_youtube_com_by.json',
              'domain_reliability/baked_in_configs/star_youtube_com_co.json',
              'domain_reliability/baked_in_configs/star_youtube_com_do.json',
              'domain_reliability/baked_in_configs/star_youtube_com_ee.json',
              'domain_reliability/baked_in_configs/star_youtube_com_eg.json',
              'domain_reliability/baked_in_configs/star_youtube_com_es.json',
              'domain_reliability/baked_in_configs/star_youtube_com_gh.json',
              'domain_reliability/baked_in_configs/star_youtube_com_gr.json',
              'domain_reliability/baked_in_configs/star_youtube_com_gt.json',
              'domain_reliability/baked_in_configs/star_youtube_com_hk.json',
              'domain_reliability/baked_in_configs/star_youtube_com_hr.json',
              'domain_reliability/baked_in_configs/star_youtube_com_jm.json',
              'domain_reliability/baked_in_configs/star_youtube_com_jo.json',
              'domain_reliability/baked_in_configs/star_youtube_com.json',
              'domain_reliability/baked_in_configs/star_youtube_com_kw.json',
              'domain_reliability/baked_in_configs/star_youtube_com_lb.json',
              'domain_reliability/baked_in_configs/star_youtube_com_lv.json',
              'domain_reliability/baked_in_configs/star_youtube_com_mk.json',
              'domain_reliability/baked_in_configs/star_youtube_com_mt.json',
              'domain_reliability/baked_in_configs/star_youtube_com_mx.json',
              'domain_reliability/baked_in_configs/star_youtube_com_my.json',
              'domain_reliability/baked_in_configs/star_youtube_com_ng.json',
              'domain_reliability/baked_in_configs/star_youtube_com_om.json',
              'domain_reliability/baked_in_configs/star_youtube_com_pe.json',
              'domain_reliability/baked_in_configs/star_youtube_com_ph.json',
              'domain_reliability/baked_in_configs/star_youtube_com_pk.json',
              'domain_reliability/baked_in_configs/star_youtube_com_pt.json',
              'domain_reliability/baked_in_configs/star_youtube_com_qa.json',
              'domain_reliability/baked_in_configs/star_youtube_com_ro.json',
              'domain_reliability/baked_in_configs/star_youtube_com_sa.json',
              'domain_reliability/baked_in_configs/star_youtube_com_sg.json',
              'domain_reliability/baked_in_configs/star_youtube_com_tn.json',
              'domain_reliability/baked_in_configs/star_youtube_com_tr.json',
              'domain_reliability/baked_in_configs/star_youtube_com_tw.json',
              'domain_reliability/baked_in_configs/star_youtube_com_ua.json',
              'domain_reliability/baked_in_configs/star_youtube_com_uy.json',
              'domain_reliability/baked_in_configs/star_youtube_com_ve.json',
              'domain_reliability/baked_in_configs/star_youtube_co_nz.json',
              'domain_reliability/baked_in_configs/star_youtube_co_th.json',
              'domain_reliability/baked_in_configs/star_youtube_co_ug.json',
              'domain_reliability/baked_in_configs/star_youtube_co_uk.json',
              'domain_reliability/baked_in_configs/star_youtube_co_ve.json',
              'domain_reliability/baked_in_configs/star_youtube_co_za.json',
              'domain_reliability/baked_in_configs/star_youtube_cz.json',
              'domain_reliability/baked_in_configs/star_youtube_de.json',
              'domain_reliability/baked_in_configs/star_youtube_dk.json',
              'domain_reliability/baked_in_configs/star_youtubeeducation_com.json',
              'domain_reliability/baked_in_configs/star_youtube_ee.json',
              'domain_reliability/baked_in_configs/star_youtube_es.json',
              'domain_reliability/baked_in_configs/star_youtube_fi.json',
              'domain_reliability/baked_in_configs/star_youtube_fr.json',
              'domain_reliability/baked_in_configs/star_youtube_ge.json',
              'domain_reliability/baked_in_configs/star_youtube_gr.json',
              'domain_reliability/baked_in_configs/star_youtube_gt.json',
              'domain_reliability/baked_in_configs/star_youtube_hk.json',
              'domain_reliability/baked_in_configs/star_youtube_hr.json',
              'domain_reliability/baked_in_configs/star_youtube_hu.json',
              'domain_reliability/baked_in_configs/star_youtube_ie.json',
              'domain_reliability/baked_in_configs/star_youtube_in.json',
              'domain_reliability/baked_in_configs/star_youtube_is.json',
              'domain_reliability/baked_in_configs/star_youtube_it.json',
              'domain_reliability/baked_in_configs/star_youtube_jo.json',
              'domain_reliability/baked_in_configs/star_youtube_jp.json',
              'domain_reliability/baked_in_configs/star_youtu_be.json',
              'domain_reliability/baked_in_configs/star_youtube_kr.json',
              'domain_reliability/baked_in_configs/star_youtube_lk.json',
              'domain_reliability/baked_in_configs/star_youtube_lt.json',
              'domain_reliability/baked_in_configs/star_youtube_lv.json',
              'domain_reliability/baked_in_configs/star_youtube_ma.json',
              'domain_reliability/baked_in_configs/star_youtube_md.json',
              'domain_reliability/baked_in_configs/star_youtube_me.json',
              'domain_reliability/baked_in_configs/star_youtube_mk.json',
              'domain_reliability/baked_in_configs/star_youtubemobilesupport_com.json',
              'domain_reliability/baked_in_configs/star_youtube_mx.json',
              'domain_reliability/baked_in_configs/star_youtube_my.json',
              'domain_reliability/baked_in_configs/star_youtube_ng.json',
              'domain_reliability/baked_in_configs/star_youtube_nl.json',
              'domain_reliability/baked_in_configs/star_youtube-nocookie_com.json',
              'domain_reliability/baked_in_configs/star_youtube_no.json',
              'domain_reliability/baked_in_configs/star_youtube_pe.json',
              'domain_reliability/baked_in_configs/star_youtube_ph.json',
              'domain_reliability/baked_in_configs/star_youtube_pk.json',
              'domain_reliability/baked_in_configs/star_youtube_pl.json',
              'domain_reliability/baked_in_configs/star_youtube_pr.json',
              'domain_reliability/baked_in_configs/star_youtube_pt.json',
              'domain_reliability/baked_in_configs/star_youtube_qa.json',
              'domain_reliability/baked_in_configs/star_youtube_ro.json',
              'domain_reliability/baked_in_configs/star_youtube_rs.json',
              'domain_reliability/baked_in_configs/star_youtube_ru.json',
              'domain_reliability/baked_in_configs/star_youtube_sa.json',
              'domain_reliability/baked_in_configs/star_youtube_se.json',
              'domain_reliability/baked_in_configs/star_youtube_sg.json',
              'domain_reliability/baked_in_configs/star_youtube_si.json',
              'domain_reliability/baked_in_configs/star_youtube_sk.json',
              'domain_reliability/baked_in_configs/star_youtube_sn.json',
              'domain_reliability/baked_in_configs/star_youtube_tn.json',
              'domain_reliability/baked_in_configs/star_youtube_ua.json',
              'domain_reliability/baked_in_configs/star_youtube_ug.json',
              'domain_reliability/baked_in_configs/star_youtube_uy.json',
              'domain_reliability/baked_in_configs/star_youtube_vn.json',
              'domain_reliability/baked_in_configs/star_ytimg_com.json',
              'domain_reliability/baked_in_configs/t0_gstatic_com.json',
              'domain_reliability/baked_in_configs/t1_gstatic_com.json',
              'domain_reliability/baked_in_configs/t2_gstatic_com.json',
              'domain_reliability/baked_in_configs/t3_gstatic_com.json',
              'domain_reliability/baked_in_configs/themes_googleusercontent_com.json',
              'domain_reliability/baked_in_configs/www_googleadservices_com.json',
              'domain_reliability/baked_in_configs/www_google_com.json',
              'domain_reliability/baked_in_configs/www_gstatic_com.json',
              'domain_reliability/baked_in_configs/www_youtube_com.json',
            ],
          },
          'inputs': [
            '<(bake_in_configs_script)',
            '<@(baked_in_configs)',
          ],
          'outputs': [
            '<(baked_in_configs_cc)'
          ],
          # The actual list of JSON files will overflow the command line length
          # limit on Windows, so pass the name of the .gypi file and
          # bake_in_configs.py will read the filenames out of it manually.
          'action': ['python',
                     '<(bake_in_configs_script)',
                     'domain_reliability.gypi',
                     '<(baked_in_configs_cc)'],
          'process_outputs_as_sources': 1,
          'message': 'Baking in Domain Reliability configs',
        },
      ],
    },
  ],
}
