/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "netd.h"

/**
 * Create a new VRF
 * @param state Server state
 * @param name VRF name
 * @param fib_number FIB number
 * @return 0 on success, -1 on failure
 */
int vrf_create(netd_state_t *state, const char *name, uint32_t fib_number)
{
    vrf_t *vrf;

    if (!state || !name || !is_valid_vrf_name(name) || !is_valid_fib_number(fib_number)) {
        debug_log(DEBUG_ERROR, "Invalid parameters for VRF creation");
        return -1;
    }

    /* Check if VRF already exists */
    if (vrf_find_by_name(state, name)) {
        debug_log(DEBUG_ERROR, "VRF %s already exists", name);
        return -1;
    }

    if (vrf_find_by_fib(state, fib_number)) {
        debug_log(DEBUG_ERROR, "FIB %u already assigned to another VRF", fib_number);
        return -1;
    }

    /* Allocate new VRF */
    vrf = malloc(sizeof(*vrf));
    if (!vrf) {
        debug_log(DEBUG_ERROR, "Failed to allocate memory for VRF");
        return -1;
    }

    /* Initialize VRF */
    memset(vrf, 0, sizeof(*vrf));
    strlcpy(vrf->name, name, sizeof(vrf->name));
    vrf->fib_number = fib_number;

    /* Add to VRF list */
    TAILQ_INSERT_TAIL(&state->vrfs, vrf, entries);

    debug_log(DEBUG_INFO, "Created VRF %s with FIB %u", name, fib_number);
    return 0;
}

/**
 * Delete a VRF
 * @param state Server state
 * @param name VRF name
 * @return 0 on success, -1 on failure
 */
int vrf_delete(netd_state_t *state, const char *name)
{
    vrf_t *vrf;
    interface_t *iface;

    if (!state || !name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for VRF deletion");
        return -1;
    }

    /* Cannot delete default VRF */
    if (strcmp(name, "default") == 0) {
        debug_log(DEBUG_ERROR, "Cannot delete default VRF");
        return -1;
    }

    /* Find VRF */
    vrf = vrf_find_by_name(state, name);
    if (!vrf) {
        debug_log(DEBUG_ERROR, "VRF %s not found", name);
        return -1;
    }

    /* Remove all interfaces from this VRF */
    TAILQ_FOREACH(iface, &state->interfaces, entries) {
        if (iface->fib == vrf->fib_number) {
            iface->fib = 0; /* Move to default VRF */
            debug_log(DEBUG_DEBUG, "Moved interface %s to default VRF", iface->name);
        }
    }

    /* Flush all routes for this FIB */
    route_flush_fib(state, vrf->fib_number);

    /* Remove from VRF list */
    TAILQ_REMOVE(&state->vrfs, vrf, entries);
    free(vrf);

    debug_log(DEBUG_INFO, "Deleted VRF %s", name);
    return 0;
}

/**
 * Find VRF by name
 * @param state Server state
 * @param name VRF name
 * @return VRF structure or NULL if not found
 */
vrf_t *vrf_find_by_name(netd_state_t *state, const char *name)
{
    vrf_t *vrf;

    if (!state || !name) {
        return NULL;
    }

    TAILQ_FOREACH(vrf, &state->vrfs, entries) {
        if (strcmp(vrf->name, name) == 0) {
            return vrf;
        }
    }

    return NULL;
}

/**
 * Find VRF by FIB number
 * @param state Server state
 * @param fib_number FIB number
 * @return VRF structure or NULL if not found
 */
vrf_t *vrf_find_by_fib(netd_state_t *state, uint32_t fib_number)
{
    vrf_t *vrf;

    if (!state) {
        return NULL;
    }

    TAILQ_FOREACH(vrf, &state->vrfs, entries) {
        if (vrf->fib_number == fib_number) {
            return vrf;
        }
    }

    return NULL;
}

/**
 * List all VRFs
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int vrf_list(netd_state_t *state)
{
    vrf_t *vrf;
    int count = 0;

    if (!state) {
        return -1;
    }

    debug_log(DEBUG_INFO, "Listing VRFs");

    /* Always show default VRF */
    debug_log(DEBUG_INFO, "  default (FIB 0)");
    count++;

    TAILQ_FOREACH(vrf, &state->vrfs, entries) {
        debug_log(DEBUG_INFO, "  %s (FIB %u)", vrf->name, vrf->fib_number);
        if (vrf->description[0] != '\0') {
            debug_log(DEBUG_INFO, "    Description: %s", vrf->description);
        }
        count++;
    }

    debug_log(DEBUG_INFO, "Total VRFs: %d", count);
    return 0;
}

/**
 * Get all VRFs as XML for NETCONF response
 * @param state Server state
 * @return XML string (allocated) or NULL on failure
 */
char *vrf_get_all(netd_state_t *state)
{
    vrf_t *vrf;
    char *xml = NULL;
    char *temp_xml = NULL;
    
    if (!state) {
        return NULL;
    }

    /* Start XML */
    asprintf(&xml, "    <vrfs xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">\n");
    if (!xml) {
        return NULL;
    }

    /* Always include default VRF */
    asprintf(&temp_xml,
            "      <vrf>\n"
            "        <name>default</name>\n"
            "        <description>Default VRF (system managed)</description>\n"
            "      </vrf>\n");
    if (temp_xml) {
        char *new_xml;
        asprintf(&new_xml, "%s%s", xml, temp_xml);
        free(xml);
        free(temp_xml);
        xml = new_xml;
    }

    TAILQ_FOREACH(vrf, &state->vrfs, entries) {
        asprintf(&temp_xml,
                "      <vrf>\n"
                "        <name>%s</name>\n"
                "        <description>%s</description>\n"
                "      </vrf>\n",
                vrf->name,
                vrf->description[0] != '\0' ? vrf->description : "");

        if (temp_xml) {
            char *new_xml;
            asprintf(&new_xml, "%s%s", xml, temp_xml);
            free(xml);
            free(temp_xml);
            xml = new_xml;
        }
    }

    /* Close XML tags */
    asprintf(&temp_xml, "    </vrfs>\n");
    if (temp_xml) {
        char *new_xml;
        asprintf(&new_xml, "%s%s", xml, temp_xml);
        free(xml);
        free(temp_xml);
        xml = new_xml;
    }

    return xml;
} 