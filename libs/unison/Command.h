/*
 * Command.h
 *
 * Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
 *
 * This file is part of Unison - http://unison.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */


#ifndef UNISON_COMMAND_H
#define UNISON_COMMAND_H

namespace Unison {

class ProcessingContext;

/**
 * Command is an implementation of the GoF Command pattern designed to
 * help with RT compliance.  This allows the client to instantiate a
 * command, then allows it to be executed across multiple contexts:
 * non-RT for setup, RT for execution, non-RT for tear-down.  Commands
 * are allocated and queued in RingBuffer for syscall-free and lockless
 * execution. */
class Command
{
  public:
    /**
     * The lifecyle of a Command */
    enum State {
      Invalid,      ///< The command is somehow deemed invalid
      Created,      ///< The command has been created but not processed
      PreExecuted,  ///< The command has completed pre-execution
      Executed,     ///< The command has completed execution
      PostExecuted  ///< The command has completed post-execution
    };

    Command (/* TODO: Going to need some parameters here */);
    virtual ~Command ()
    {};

    /**
     * preExecute is for preparing the command to be executed.  This function
     * is called outside of the processing thread.  Therefore, it is safe to
     * call non-realtime-safe stuff here. */
    virtual void preExecute ();

    /**
     * execute is where the Command should basically "commit" the changes.
     * execute is called from the processing thread.  Therefore, it must be
     * realtime-safe.  Commands are executed at the beginning of the Backend's
     * processing function.  This allows for a chance to synchronize the
     * changes influenced by the Command. */
    virtual void execute (ProcessingContext &context);

    /**
     * postExecute is where the Command can cleanup after itself.  It is
     * invoked after preExecute.  It should be safe to call non-RT stuff
     * here.
     * TODO: Figure out where/how to call this function. Another thread? */
    virtual void postExecute ();

    /**
     * Is this command supposed to block?  If so, the dispatcher blocks until
     * the Command fails or completes post-execution
     * XXX: Maybe unblock after execution and let post-execution dangle */
    bool isBlocking () const;

    /**
     * The current lifecycle state of the Command */
    State state() const;

    /**
     * Has the command been flagged as erroneous? */
    bool hasError() const;

    /**
     * The error-code of this command. 0 is reserved for success, any other
     * value is specific to the subclass of command */
    int  errorCode() const;

  protected:
    bool m_blocking;
    State m_state;
    int m_errorCode;
};

} // Unison

#endif

// vim: ts=8 sw=2 sts=2 et sta noai