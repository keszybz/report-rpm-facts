/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <assert.h>

#include "common.h"
#include "metrics.h"
#include "varlink-io.systemd.Metrics.h"

int metrics_add_to_varlink_server(
                sd_varlink_server *server,
                sd_varlink_method_t vl_method_list_cb,
                sd_varlink_method_t vl_method_describe_cb) {

        int r;

        assert(server);
        assert(vl_method_list_cb);
        assert(vl_method_describe_cb);

        r = sd_varlink_server_add_interface(server, &vl_interface_io_systemd_Metrics);
        if (r < 0)
                return log_debug_errno(r, "Failed to add varlink metrics interface to varlink server: %m");

        r = sd_varlink_server_bind_method_many(
                        server,
                        "io.systemd.Metrics.List",     vl_method_list_cb,
                        "io.systemd.Metrics.Describe", vl_method_describe_cb);
        if (r < 0)
                return log_debug_errno(r, "Failed to register varlink metrics methods: %m");

        return 0;
}

static const char* metric_family_type_to_string(MetricFamilyType type) {
        switch (type) {
        case METRIC_FAMILY_TYPE_COUNTER: return "counter";
        case METRIC_FAMILY_TYPE_GAUGE:   return "gauge";
        case METRIC_FAMILY_TYPE_STRING:  return "string";
        default: return NULL;
        }
}

static int metric_family_build_json(const MetricFamily *mf, sd_json_variant **ret) {
        assert(mf);

        return sd_json_buildo(
                        ret,
                        SD_JSON_BUILD_PAIR_STRING("name", mf->name),
                        SD_JSON_BUILD_PAIR_STRING("description", mf->description),
                        SD_JSON_BUILD_PAIR_STRING("type", metric_family_type_to_string(mf->type)));
}

int metrics_method_describe(
                const MetricFamily metric_family_table[],
                sd_varlink *link,
                sd_json_variant *parameters,
                sd_varlink_method_flags_t flags,
                _unused_ void *userdata) {

        int r;

        assert(metric_family_table);
        assert(link);
        assert(parameters);
        assert(flags & SD_VARLINK_METHOD_MORE);

        r = sd_varlink_dispatch(link, parameters, /* dispatch_table= */ NULL, /* userdata= */ NULL);
        if (r != 0)
                return r;

        r = sd_varlink_set_sentinel(link, "io.systemd.Metrics.NoSuchMetric");
        if (r < 0)
                return r;

        for (const MetricFamily *mf = metric_family_table; mf && mf->name; mf++) {
                _cleanup_(sd_json_variant_unrefp) sd_json_variant *v = NULL;

                r = metric_family_build_json(mf, &v);
                if (r < 0)
                        return log_debug_errno(r, "Failed to describe metric family '%s': %m", mf->name);

                r = sd_varlink_reply(link, v);
                if (r < 0)
                        return log_debug_errno(r, "Failed to send varlink reply: %m");
        }

        return 0;
}

int metrics_method_list(
                const MetricFamily metric_family_table[],
                sd_varlink *link,
                sd_json_variant *parameters,
                sd_varlink_method_flags_t flags,
                void *userdata) {

        int r;

        assert(metric_family_table);
        assert(link);
        assert(parameters);
        assert(flags & SD_VARLINK_METHOD_MORE);

        r = sd_varlink_dispatch(link, parameters, /* dispatch_table= */ NULL, /* userdata= */ NULL);
        if (r != 0)
                return r;

        r = sd_varlink_set_sentinel(link, "io.systemd.Metrics.NoSuchMetric");
        if (r < 0)
                return r;

        MetricFamilyContext ctx = { .link = link };
        for (const MetricFamily *mf = metric_family_table; mf && mf->name; mf++) {
                assert(mf->generate);

                ctx.metric_family = mf;
                r = mf->generate(&ctx, userdata);
                if (r < 0)
                        return log_debug_errno(
                                        r, "Failed to list metrics for metric family '%s': %m", mf->name);
        }

        return 0;
}

static int metric_build_send(MetricFamilyContext *context, const char *object, sd_json_variant *value, sd_json_variant *fields) {
        assert(context);
        assert(value);
        assert(context->link);
        assert(context->metric_family);
        assert(!object); /* not implemented */
        assert(!fields); /* not implemented */

        return sd_varlink_replybo(context->link,
                                  SD_JSON_BUILD_PAIR_STRING("name", context->metric_family->name),
                                  SD_JSON_BUILD_PAIR_VARIANT("value", value));
}

int metric_build_send_string(MetricFamilyContext *context, const char *object, const char *value, sd_json_variant *fields) {
        _cleanup_(sd_json_variant_unrefp) sd_json_variant *v = NULL;
        int r;

        assert(value);

        r = sd_json_variant_new_string(&v, value);
        if (r < 0)
                return log_debug_errno(r, "Failed to allocate JSON string: %m");

        return metric_build_send(context, object, v, fields);
}

int metric_build_send_unsigned(MetricFamilyContext *context, const char *object, uint64_t value, sd_json_variant *fields) {
        _cleanup_(sd_json_variant_unrefp) sd_json_variant *v = NULL;
        int r;

        r = sd_json_variant_new_unsigned(&v, value);
        if (r < 0)
                return log_debug_errno(r, "Failed to allocate JSON unsigned: %m");

        return metric_build_send(context, object, v, fields);
}
