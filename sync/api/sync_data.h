// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNC_API_SYNC_DATA_H_
#define SYNC_API_SYNC_DATA_H_

#include <iosfwd>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/stl_util.h"
#include "base/time/time.h"
#include "sync/api/attachments/attachment.h"
#include "sync/api/attachments/attachment_service_proxy.h"
#include "sync/base/sync_export.h"
#include "sync/internal_api/public/base/model_type.h"
#include "sync/internal_api/public/util/immutable.h"
#include "sync/internal_api/public/util/weak_handle.h"

namespace sync_pb {
class EntitySpecifics;
class SyncEntity;
}  // namespace sync_pb

namespace syncer {

class AttachmentService;

// A light-weight container for immutable sync data. Pass-by-value and storage
// in STL containers are supported and encouraged if helpful.
class SYNC_EXPORT SyncData {
 public:
  // Creates an empty and invalid SyncData.
  SyncData();
  ~SyncData();

  // Default copy and assign welcome.

  // Helper methods for creating SyncData objects for local data.
  // The sync tag must be a string unique to this datatype and is used as a node
  // identifier server-side.
  // For deletes: |datatype| must specify the datatype who node is being
  // deleted.
  // For adds/updates: the specifics must be valid and the non-unique title (can
  // be the same as sync tag) must be specfied.
  // Note: the non_unique_title is primarily for debug purposes, and will be
  // overwritten if the datatype is encrypted.
  static SyncData CreateLocalDelete(
      const std::string& sync_tag,
      ModelType datatype);
  static SyncData CreateLocalData(
      const std::string& sync_tag,
      const std::string& non_unique_title,
      const sync_pb::EntitySpecifics& specifics);
  static SyncData CreateLocalDataWithAttachments(
      const std::string& sync_tag,
      const std::string& non_unique_title,
      const sync_pb::EntitySpecifics& specifics,
      const AttachmentList& attachments);

  // Helper method for creating SyncData objects originating from the syncer.
  //
  // TODO(maniscalco): Replace all calls to 3-arg CreateRemoteData with calls to
  // the 5-arg version (bug 353296).
  static SyncData CreateRemoteData(
      int64 id,
      const sync_pb::EntitySpecifics& specifics,
      const base::Time& last_modified_time,
      const AttachmentIdList& attachment_ids,
      const syncer::AttachmentServiceProxy& attachment_service);
  static SyncData CreateRemoteData(int64 id,
                                   const sync_pb::EntitySpecifics& specifics,
                                   const base::Time& last_modified_time);

  // Whether this SyncData holds valid data. The only way to have a SyncData
  // without valid data is to use the default constructor.
  bool IsValid() const;

  // Return the datatype we're holding information about. Derived from the sync
  // datatype specifics.
  ModelType GetDataType() const;

  // Return the current sync datatype specifics.
  const sync_pb::EntitySpecifics& GetSpecifics() const;

  // Returns the value of the unique client tag. This is only set for data going
  // TO the syncer, not coming from.
  const std::string& GetTag() const;

  // Returns the non unique title (for debugging). Currently only set for data
  // going TO the syncer, not from.
  const std::string& GetTitle() const;

  // Returns the last motification time according to the server. This is
  // only valid if IsLocal() is false, and may be null if the SyncData
  // represents a deleted item.
  const base::Time& GetRemoteModifiedTime() const;

  // Should only be called by sync code when IsLocal() is false.
  int64 GetRemoteId() const;

  // Whether this sync data is for local data or data coming from the syncer.
  bool IsLocal() const;

  // TODO(maniscalco): Reduce the dependence on knowing whether a SyncData is
  // local (in the IsLocal() == true sense) or remote.  Make it harder for users
  // of SyncData to accidentally call local-only methods on a remote SyncData
  // (bug 357305).

  std::string ToString() const;

  // Return a list of this SyncData's attachment ids.
  //
  // The attachments may or may not be present on this device.
  AttachmentIdList GetAttachmentIds() const;

  // Return a list of this SyncData's attachments.
  //
  // May only be called when IsLocal() is true.
  const AttachmentList& GetLocalAttachmentsForUpload() const;

  // Retrieve the attachments indentified by |attachment_ids|. Invoke |callback|
  // with the requested attachments.
  //
  // May only be called when IsLocal() is false.
  //
  // |callback| will be invoked when the operation is complete (successfully or
  // otherwise).
  //
  // Retrieving the requested attachments may require reading local storage or
  // requesting the attachments from the network.
  //
  void GetOrDownloadAttachments(
      const AttachmentIdList& attachment_ids,
      const AttachmentService::GetOrDownloadCallback& callback);

  // Drop (delete from local storage) the attachments associated with this
  // SyncData specified in |attachment_ids|. This method will not delete
  // attachments from the server.
  //
  // May only be called when IsLocal() is false.
  //
  // |callback| will be invoked when the operation is complete (successfully or
  // otherwise).
  void DropAttachments(const AttachmentIdList& attachment_ids,
                       const AttachmentService::DropCallback& callback);

  // TODO(zea): Query methods for other sync properties: parent, successor, etc.

 private:
  // Necessary since we forward-declare sync_pb::SyncEntity; see
  // comments in immutable.h.
  struct ImmutableSyncEntityTraits {
    typedef sync_pb::SyncEntity* Wrapper;

    static void InitializeWrapper(Wrapper* wrapper);

    static void DestroyWrapper(Wrapper* wrapper);

    static const sync_pb::SyncEntity& Unwrap(const Wrapper& wrapper);

    static sync_pb::SyncEntity* UnwrapMutable(Wrapper* wrapper);

    static void Swap(sync_pb::SyncEntity* t1, sync_pb::SyncEntity* t2);
  };

  typedef Immutable<sync_pb::SyncEntity, ImmutableSyncEntityTraits>
      ImmutableSyncEntity;

  // Clears |entity| and |attachments|.
  SyncData(
      int64 id,
      sync_pb::SyncEntity* entity,
      AttachmentList* attachments,
      const base::Time& remote_modification_time,
      const syncer::AttachmentServiceProxy& attachment_service);

  // Whether this SyncData holds valid data.
  bool is_valid_;

  // Equal to kInvalidId iff this is local.
  int64 id_;

  // This is only valid if IsLocal() is false, and may be null if the
  // SyncData represents a deleted item.
  base::Time remote_modification_time_;

  // The actual shared sync entity being held.
  ImmutableSyncEntity immutable_entity_;

  Immutable<AttachmentList> attachments_;

  AttachmentServiceProxy attachment_service_;
};

// gmock printer helper.
void PrintTo(const SyncData& sync_data, std::ostream* os);

typedef std::vector<SyncData> SyncDataList;

}  // namespace syncer

#endif  // SYNC_API_SYNC_DATA_H_
