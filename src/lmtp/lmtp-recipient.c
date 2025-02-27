/* Copyright (c) 2018 Dovecot authors, see the included COPYING file */

#include "lmtp-common.h"
#include "array.h"
#include "smtp-server.h"
#include "lmtp-recipient.h"

struct lmtp_recipient_module_register
lmtp_recipient_module_register = { 0 };

struct lmtp_recipient *
lmtp_recipient_create(struct client *client,
		      struct smtp_server_transaction *trans,
		      struct smtp_server_recipient *rcpt)
{
	struct lmtp_recipient *lrcpt;

	lrcpt = p_new(rcpt->pool, struct lmtp_recipient, 1);
	lrcpt->rcpt = rcpt;	
	lrcpt->client = client;

	rcpt->context = lrcpt;

	p_array_init(&lrcpt->module_contexts, rcpt->pool, 5);

	/* Use a unique session_id for each mail delivery. This is especially
	   important for stats process to not see duplicate sessions. */
	client->state.session_id_seq++;
	lrcpt->session_id = p_strdup_printf(rcpt->pool, "%s:%u", trans->id,
					    client->state.session_id_seq);

	event_add_str(rcpt->event, "session", lrcpt->session_id);

	return lrcpt;
}

struct lmtp_recipient *
lmtp_recipient_find_duplicate(struct lmtp_recipient *lrcpt,
			      struct smtp_server_transaction *trans)
{
	struct smtp_server_recipient *drcpt;
	struct lmtp_recipient *dup_lrcpt;

	i_assert(lrcpt->rcpt != NULL);
	drcpt = smtp_server_transaction_find_rcpt_duplicate(trans, lrcpt->rcpt);
	if (drcpt == NULL)
		return NULL;

	dup_lrcpt = drcpt->context;
	i_assert(dup_lrcpt->rcpt == drcpt);
	i_assert(dup_lrcpt->type == lrcpt->type);

	return dup_lrcpt;
}

