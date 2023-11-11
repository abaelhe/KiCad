/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2023 KiCad Developers, see AUTHORS.TXT for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

namespace KIGIT_ERROR
{
    #undef _
    #define _(a) a

        // General errors
    const char* const kInvalidRepository = _("Invalid repository.");
    const char* const kCommitFailed = _("Failed to commit changes.");
    const char* const kMergeFailed = _("Failed to merge branches.");

    // Clone errors
    const char* const kCloneFailed = _("Failed to clone repository.");
    const char* const kRemoteNotFound = _("Remote repository not found.");
    const char* const kAuthenticationFailed = _("Authentication failed for remote repository.");

    // Branch errors
    const char* const kBranchNotFound = _("Branch not found.");
    const char* const kBranchCreationFailed = _("Failed to create branch.");
    const char* const kBranchDeletionFailed = _("Failed to delete branch.");

    // Checkout errors
    const char* const kCheckoutFailed = _("Failed to perform checkout operation.");
    const char* const kFileNotFoundInCheckout = _("File not found during checkout operation.");

    // Conflict errors
    const char* const kMergeConflict = _("Merge conflict encountered.");
    const char* const kRebaseConflict = _("Rebase conflict encountered.");

    // Pull/Push errors
    const char* const kPullFailed = _("Failed to pull changes from remote repository.");
    const char* const kPushFailed = _("Failed to push changes to remote repository.");
    const char* const kNoUpstreamBranch = _("No upstream branch configured.");
    const char* const kRemoteConnectionError = _("Failed to establish connection with remote repository.");

    // Tag errors
    const char* const kTagNotFound = _("Tag not found.");
    const char* const kTagCreationFailed = _("Failed to create tag.");
    const char* const kTagDeletionFailed = _("Failed to delete tag.");

    const char* const kUnknownError = _("Unknown error.");
    const char* const kNoError = _("No error.");

}